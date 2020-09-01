#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "dvp.h"
#include "fpioa.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "utils.h"
#include "kpu.h"
#include "l_conv.h"
#include "dvp_cam.h"
#include "math.h"

//Activation function breakpoint table, set to y=x, that is, directly output the convolution result
//y=(uint8_t)((((uint64_t)(x - x_start) * y_mul) >> shift) + bias);

kpu_activate_table_t active_addr __attribute__((aligned(256))) = {
	.activate_para = {//x =36bit
					  {.data = {.shift_number = 0, .y_mul = 0, .x_start = 0x800000000}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}},
					  {.data = {.shift_number = 0, .y_mul = 1, .x_start = 0}}},
	.activate_para_bias0.data = {.result_bias = {0, 0, 0, 0, 0, 0, 0, 0}},
	.activate_para_bias1.data = {.result_bias = {0, 0, 0, 0, 0, 0, 0, 0}}};

//y = (x*norm_mul)>>norm_shift + norm_add
kpu_batchnorm_argument_t bwsx_base_addr[] __attribute__((aligned(128))) = {
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
	{.batchnorm.data = {.norm_mul = 1, .norm_add = 0x0, .norm_shift = 0}},
};

//卷积参数
kpu_layer_argument_t la __attribute__((aligned(128)));
//max for 3in*3out, you can modify it
uint16_t conv_data_u16[9 * 3 * 3] __attribute__((aligned(128)));

//Pooling type, 0 means skip
//0x1 represents 2x2 max pooling with a step size of 2,
//0x2 represents 2x2 mean pooling with a step size of 2,
//0x3 represents 4x4 max pooling with a step size of 4.
//0x4 represents 4x4 mean pooling with a step size of 4.
//0x5 represents 2x2 left_top pooling with a step size of 2,
//0x6 represents 2x2 right_bottom pooling with a step size of 2,
//0x7 represents 4x4 left_top pooling with a step size of 4,
//0x8 represents 2x2 mean pooling with a step size of 1,
//0x9 represents 2x2 max pooling with a step size of 1
#define AI_MEM_SIZE 0x200000

static float min(float *data, uint32_t len)
{
	int i;
	float m = data[0];
	for (i = 0; i < len; i++)
	{
		if (data[i] < m)
			m = data[i];
	}
	return m;
}

static float max(float *data, uint32_t len)
{
	int i;
	float m = data[0];
	for (i = 0; i < len; i++)
	{
		if (data[i] > m)
			m = data[i];
	}
	return m;
}

//global var: la, active_addr, bwsx_base_addr
static void conv_float2u16(float *data, uint16_t *data_u16, int len)
{
	float dmin, drange, scale, arg_x;
	uint16_t bias, y_mul;
	int i, shift_number;
	dmin = min(data, len);
	drange = max(data, len) - dmin;
	scale = (65535.0 / drange);
	//scale conv
	printf("convert conv parm: -------------\r\n");
	for (i = 0; i < len; i++)
	{
		data_u16[i] = (uint16_t)((data[i] - dmin) * scale);
		printf("0x%04x\t", data_u16[i]);
		if (i % 9 == 8)
			printf("\r\n");
	}
	printf("set arg_x & shr_x: -------------\r\n");
	arg_x = scale * (dmin >= 0 ? dmin : -dmin);
	for (i = 0; (arg_x < (float)(0x400000)) && (arg_x != 0); i++)
	{
		arg_x *= 2;
	}
	la.conv_value.data.arg_x = dmin >= 0 ? (uint32_t)(arg_x) : (uint32_t)(0x1000000 - (uint32_t)arg_x);
	la.conv_value.data.shr_x = i;
	printf("arg_x=0x%x, shr_x=%d\r\n", la.conv_value.data.arg_x, la.conv_value.data.shr_x);
	//set act table
	printf("set act table: -------------\r\n");
	printf("origin scale=%f\r\n", scale);
	scale = 1.0 / scale;
	for (i = 0; scale <= 16383.0; i++)
	{
		scale = scale * 2;
	}
	shift_number = i;
	y_mul = (uint16_t)(scale);
	printf("shift_number=%d, y_mul=%d\r\n", shift_number, y_mul);
	for (i = 1; i < 16; i++)
	{
		active_addr.activate_para[i].data.shift_number = shift_number;
		active_addr.activate_para[i].data.y_mul = y_mul;
		active_addr.activate_para[i].data.x_start = 0;
	}
	return;
}

void conv_init(kpu_task_t *task, convolution_kernel_t conv_x_x, float *conv_data)
{
	int tmp;
	uint8_t ch_in = sqrt(conv_x_x);
	uint8_t ch_out = ch_in;

	conv_float2u16(conv_data, conv_data_u16, 9 * ch_in * ch_out); //3x3 kernel

	la.kernel_offset.data.coef_row_offset = 0;	//Fixed at 0
	la.kernel_offset.data.coef_column_offset = 0; //Fixed at 0
	//Activation function configuration
	la.kernel_calc_type_cfg.data.load_act = 1; //Enable activation function
	la.kernel_calc_type_cfg.data.active_addr = (uint64_t)&active_addr;
	//Initialize activation table
	//row_switch_addr = math.ceil(i_row_wid / 64)
	//channel_switch_addr = i_col_high * row_switch_addr
	la.kernel_calc_type_cfg.data.row_switch_addr = (CAM_WIDTH_PIXEL + 63) / 64; //The number of units occupied by the image width
	la.kernel_calc_type_cfg.data.channel_switch_addr = (CAM_WIDTH_PIXEL + 63) / 64 * CAM_HIGHT_PIXEL;
	la.kernel_calc_type_cfg.data.coef_size = 0; //Fixed at 0
	la.kernel_calc_type_cfg.data.coef_group = 1;

	//Interrupt settings
	la.interrupt_enabe.data.depth_wise_layer = 0; //Conventional convolutional layer
	la.interrupt_enabe.data.int_en = 1;			  //Enable interrupt
	la.interrupt_enabe.data.full_add = 0;		  
	la.interrupt_enabe.data.ram_flag = 1;		  
	//DMA setting, know that it is the DMA used for output data
	la.dma_parameter.data.dma_total_byte = CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL * ch_out - 1; //Total number of DMA transfers
	la.dma_parameter.data.send_data_out = 1;				   //Enable data dma output
	la.dma_parameter.data.channel_byte_num = CAM_WIDTH_PIXEL * CAM_HIGHT_PIXEL - 1;		   //Number of DMA transfers per channel
	//Convolution operation parameter setting

	//Write back settings
	la.write_back_cfg.data.wb_row_switch_addr = (CAM_WIDTH_PIXEL + 63) / 64;		   //ceil(16/64)=1
	la.write_back_cfg.data.wb_channel_switch_addr = (CAM_WIDTH_PIXEL + 63) / 64 * CAM_HIGHT_PIXEL; //16*1
	la.write_back_cfg.data.wb_group = 1;							   //64/w
	//Image size setting
	la.image_size.data.i_row_wid = CAM_WIDTH_PIXEL - 1; 
	la.image_size.data.i_col_high = CAM_HIGHT_PIXEL - 1;
	la.image_size.data.o_row_wid = CAM_WIDTH_PIXEL - 1; 
	la.image_size.data.o_col_high = CAM_HIGHT_PIXEL - 1;
	//Pool type Settings
	la.kernel_pool_type_cfg.data.bypass_conv = 0;	 
	la.kernel_pool_type_cfg.data.pad_value = 0x0;	 
	la.kernel_pool_type_cfg.data.load_para = 1;		  
	la.kernel_pool_type_cfg.data.pad_type = 0;		  
	la.kernel_pool_type_cfg.data.kernel_type = 1;	 //3x3
	la.kernel_pool_type_cfg.data.pool_type = 0;		  
	la.kernel_pool_type_cfg.data.dma_burst_size = 15; //Dma burst transfer size, 16 bytes
	la.kernel_pool_type_cfg.data.bwsx_base_addr = (uint64_t)&bwsx_base_addr;
	//Batch normalization initial address
	la.kernel_pool_type_cfg.data.first_stride = CAM_HIGHT_PIXEL < 256 ? 0 : 1; //Image height does not exceed 255
	//Image channel Settings
	la.image_channel_num.data.o_ch_num_coef = ch_out - 1; //The number of channels that can be calculated by one-time parameter loading
	la.image_channel_num.data.i_ch_num = ch_in - 1;		  //Input channel
	la.image_channel_num.data.o_ch_num = ch_out - 1;	  //Output channel
	//Convolution parameter Settings 
	la.kernel_load_cfg.data.load_time = 0;						//Convolution loading times, no more than 72KB, only once
	la.kernel_load_cfg.data.para_size = 2 * 9 * ch_in * ch_out; //Convolution parameter size
	la.kernel_load_cfg.data.para_start_addr = (uint64_t)conv_data_u16;
	//Starting address
	la.kernel_load_cfg.data.load_coor = 1; //Allows loading of convolution parameters
	//Calculated address setting
	la.image_addr.data.image_src_addr = (uint64_t)0x0; //0
	la.image_addr.data.image_dst_addr = (uint64_t)(AI_MEM_SIZE / 64 - (CAM_WIDTH_PIXEL + 63) / 64 * CAM_HIGHT_PIXEL * ch_out);

	/* Initialize the KPU task */
	task->layers = &la;
	task->layers_length = 1;  //A single layer
	task->eight_bit_mode = 0; //16bit mode
	task->output_scale = 1.0; //Output scaling
	task->output_bias = 0;	//Output bias
	return;
}

void conv_run(kpu_task_t *task, uint8_t *img_src, uint8_t *img_dst, plic_irq_callback_t callback)
{
	/* KPU start run */
	kpu_run(task, DMAC_CHANNEL5, img_src, img_dst, callback);
	return;
}
