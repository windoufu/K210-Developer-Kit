#include "string.h"
#include "bsp_recorder.h"
#include "bsp_sdcard.h"
#include "sleep.h"
#include "iomem.h"
#include "iomem.h"
#include "wav_decode.h"
#include "i2s.h"
#include "rgb.h"
#include "sysctl.h"

#define MIC_I2S_DMA_CHANNEL             DMAC_CHANNEL1


#define MIC_GAIN 10
#define FRAME_LEN 512

//int16_t rx_buf[FRAME_LEN * 2 * 2];
//uint32_t g_rx_dma_buf[FRAME_LEN * 2 * 2];
int16_t *rx_buf;
uint32_t *g_rx_dma_buf;


uint32_t g_index = 0, g_record_times = 0;
int g_record_pos = 0;


uint8_t i2s_rec_flag = 0, g_recorder_switch = 0;
int sample_rate = 22050; // 44100;     //22050;
uint32_t sectorsize = 0;    
wave_header_t *wavhead = 0; 
FIL *f_rec = 0;



void bsp_recorder_deinit(void);



wave_header_t *creat_wave_header(uint32_t length, uint32_t sample_rate, uint8_t bits_perSample, uint8_t num_chans)
{
    wave_header_t *header = (wave_header_t *)iomem_malloc(sizeof(wave_header_t)+4);
    if(header == NULL)
    {
        printf("header iomem_malloc err!\r\n");
        return NULL;
    }
        
    // set all the ASCII literals
    memcpy(header->chunkID, "RIFF", 4);
    header->chunkSize = length + (44 - 8);
    memcpy(header->format, "WAVE", 4);

    memcpy(header->subchunk1ID, "fmt ", 4);
    header->subchunk1Size = 16;
    header->audioFormat = 1;
    header->numChannels = num_chans;  // 2
    header->sampleRate = sample_rate; // 22050 or 44100
    header->bitsPerSample = bits_perSample; // 16
    header->blockAlign = header->numChannels * header->bitsPerSample / 8; // 4
    header->byteRate = header->sampleRate * header->blockAlign; // 88200 or 176.4k

    memcpy(header->subchunk2ID, "data", 4);
    header->subchunk2Size = length;

    return header;
}

/* Update data size */
void update_length(wave_header_t *header, uint32_t length_bytes)
{
    header->subchunk2Size = length_bytes;
    header->chunkSize = header->subchunk2Size + 36;
}


void recoder_get_last_pathname(uint8_t *pname) 
{	
    FIL ftemp; 
	uint8_t res;					 
	uint16_t index=1;
	while(index<0XFFFF)
	{
        sprintf((char*)pname, "0:REC%05d.wav", index);
		res=f_open(&ftemp, (const TCHAR*)pname, FA_READ);
		if(res==FR_NO_FILE)
		{
			sprintf((char*)pname, "0:REC%05d.wav", index-1);
			break;	
		}	
		index++;
	}
}


void recoder_new_pathname(uint8_t *pname)
{	
    FIL ftemp; 
	uint8_t res;					 
	uint16_t index=0;
	while(index<0XFFFF)
	{
        sprintf((char*)pname, "0:REC%03d.wav", index);
		res = f_open(&ftemp, (const TCHAR*)pname, FA_READ);
		if(res==FR_NO_FILE) break;                         
		index++;
	}
}


bool recoder_pathname_success(TCHAR *pname)
{	
    FIL ftemp; 
	uint8_t res;					 
	res = f_open(&ftemp, (const TCHAR*)pname, FA_READ);
	if(res == FR_NO_FILE) 
    {
        f_close(&ftemp);
        return true;
    }
    else if(res == FR_OK)
    {
        if(f_unlink(pname) == FR_OK)
        {
            printf("del success!\r\n");
        }
        else
        {
            printf("del fail!\r\n");
            return false;
        }
        
        return true;
    }  
    else
    {
        f_close(&ftemp);
    }
                           
}


static int on_record_dma_cb(void *ctx)
{
    uint32_t i, bw;
    int16_t temp;
    //int16_t* p_rx_buf = rx_buf;
    //uint32_t p_g_rx_dma_buf = g_rx_dma_buf;
    
    //printf("fffffff\r\n");
    if(g_index == 0)
    {
       
        g_index = 1;
        for(i = 0; i < FRAME_LEN; i++)
        {
            temp =  (int16_t) ( *(g_rx_dma_buf + 2 * i + 1) * MIC_GAIN ) & 0xffff;
            //printf("temp: %d\n", temp);
            //temp = ((temp << 8) & 0xff00) | ((temp >> 8) & 0x00ff);
            //printf("temp: %d\n", temp);
            *(rx_buf + 2 * i) = temp;
            *(rx_buf + 2 * i + 1) = temp;
            //memset((int16_t *)(rx_buf + 2 * i), temp , sizeof(temp)); 
            //memset((int16_t *)(rx_buf + 2 * i + 1), temp , sizeof(temp)); 
        }

        if(g_recorder_switch == 1)
        {
            
            int res = f_write(f_rec, (const void *)rx_buf, 1024 * 2 , &bw); 
            if(res)
            {
                printf("err1:%d\r\n", res);
                printf("bw:%d\r\n", bw);
                f_close(f_rec);
                iomem_free(wavhead);
                iomem_free(f_rec);
                iomem_free(rx_buf);
                iomem_free(g_rx_dma_buf);
                bsp_recorder_deinit();
                return -1; 
            }
            sectorsize++; 
        }
        app_rgb_red_state(LIGHT_ON);
        i2s_receive_data_dma(I2S_DEVICE_0, (uint32_t *)(g_rx_dma_buf + FRAME_LEN * 2), FRAME_LEN * 2, MIC_I2S_DMA_CHANNEL);
    } 
    else
    {
        
        g_index = 0;
        for(i = FRAME_LEN; i < FRAME_LEN * 2; i++)
        {
            temp =  (int16_t)( *(g_rx_dma_buf + 2 * i + 1) * MIC_GAIN) & 0xffff;
            //temp = ((temp << 8) & 0xff00) | ((temp >> 8) & 0x00ff);
            *(rx_buf + 2 * i) = temp;
            *(rx_buf + 2 * i + 1) = temp;
            //memset((int16_t *)(rx_buf + 2 * i), temp , sizeof(temp)); 
            //memset((int16_t *)(rx_buf + 2 * i + 1), temp , sizeof(temp)); 
        }
        if(g_recorder_switch == 1)
        {
            
            int res = f_write(f_rec, (const void *)(rx_buf + FRAME_LEN * 2) , 1024 * 2, &bw); 
            if(res)
            {
                printf("err2:%d\r\n", res);
                printf("bw:%d\r\n", bw);
                f_close(f_rec);
                iomem_free(wavhead);
                iomem_free(f_rec);
                iomem_free(rx_buf);
                iomem_free(g_rx_dma_buf);
                bsp_recorder_deinit();
                return -1; 
            }
            sectorsize++; 
        }
        app_rgb_red_state(LIGHT_OFF);
        i2s_receive_data_dma(I2S_DEVICE_0, (uint32_t *)g_rx_dma_buf, FRAME_LEN * 2, MIC_I2S_DMA_CHANNEL);
    }
 
    return 0;
}

void bsp_recorder_init(void)
{
    //printk("get_free_heap_size=%ld\n", get_free_heap_size());
    rx_buf = (int16_t *)iomem_malloc(FRAME_LEN * 2 * 2 * 2);
    //printk("get_free_heap_size=%ld\n", get_free_heap_size());
    if(rx_buf == NULL )
    {
        printf("rx_buf malloc failed\r\n");
        return;
    }
    g_rx_dma_buf = (uint32_t *)iomem_malloc(FRAME_LEN * 2 * 2 * 4);
    if(g_rx_dma_buf == NULL )
    {
        printf("g_rx_dma_buf malloc failed\r\n");
        return;
    }
    //printk("get_free_heap_size=%ld\n", get_free_heap_size());

    memset((int16_t *)rx_buf, 0, FRAME_LEN * 2 * 2 );
    memset((uint32_t *)g_rx_dma_buf, 0, FRAME_LEN * 2 * 2);


    if(g_record_times == 0)
    {
        i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x03);

        i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_0,
                          RESOLUTION_16_BIT, SCLK_CYCLES_32,
                          TRIGGER_LEVEL_4, STANDARD_MODE);
        i2s_set_sample_rate(I2S_DEVICE_0, sample_rate);
        g_record_times = 1;
    }
    //recordQueue = createQueue();
    //dmac_channel_disable(MIC_I2S_DMA_CHANNEL);
    dmac_set_irq(MIC_I2S_DMA_CHANNEL, on_record_dma_cb, NULL, 1);
    i2s_receive_data_dma(I2S_DEVICE_0, (uint32_t *)(g_rx_dma_buf + g_index), FRAME_LEN * 2, MIC_I2S_DMA_CHANNEL);
   
    //i2s_handle_data_dma();
}
void bsp_recorder_deinit(void)
{
    dmac_wait_done(MIC_I2S_DMA_CHANNEL);
    //sysctl_disable_irq();
    //i2s_receive_dma_enable_pro(I2S_DEVICE_0, 0);
    dmac_channel_disable(MIC_I2S_DMA_CHANNEL);
    dmac_free_irq(MIC_I2S_DMA_CHANNEL);
    
    //sysctl_enable_irq();
    //dmac_channel_disable
    //dmac_channel_disable(MIC_I2S_DMA_CHANNEL);
    printf("bsp_recorder_deinit\r\n");
}

//Start play
void bsp_recorder_start(TCHAR *path)
{
    uint8_t res;
    uint32_t bw = 0;

    //uint8_t rec_sta = 0; 
   

    //Request a memory space of FIL bytes
    f_rec = (FIL *)iomem_malloc(sizeof(FIL));
    if(f_rec == NULL) 
    {
        printf("f_rec iomem_malloc failed\r\n");
        return;
    }
    
    printf("record is start\r\n");
    app_rgb_red_state(LIGHT_ON);
    //recoder_new_pathname(path);  //Determines whether this file name exists in the SD card, or if it does, it will be self-incrementing
    recoder_pathname_success(path);
    printf("%s\r\n", path + 2);                         //Displays the name of the current recording file
    wavhead = creat_wave_header(0, sample_rate, 16, 2); //Create wav heard file
    if(wavhead == NULL)
    {
        printf("wavhead creat failed\r\n");
        iomem_free(f_rec);
        return;
    }
    res = f_open(f_rec, (const TCHAR *)path, FA_CREATE_ALWAYS | FA_WRITE);
    if(res) 
    {
        printf("bsp_recorder_start： file open err %d\r\n", res);
        iomem_free(wavhead);
        f_close(f_rec);
        iomem_free(f_rec);
        return;     
    }
    else
    {
        res = f_write(f_rec, (const void *)wavhead, sizeof(wave_header_t), &bw); //Write header data
        if(res)
        {
            f_close(f_rec);
            printf("write wavhead fail:%d", res);
            iomem_free(wavhead);
            iomem_free(f_rec);
            return; 
        }
        printf("write wave_header first! \n");
        g_recorder_switch = 1;
    }


    bsp_recorder_init();           // Microphone initialization

    return;
}

//The recording stops and is saved
void bsp_recorder_stopsave(TCHAR *path)
{
    uint8_t res;
    uint32_t bw = 0;
    printf("1\r\n");
    if(g_recorder_switch == 1)
    {
        g_recorder_switch = 0;
        printf("2\r\n");
        bsp_recorder_deinit();
        printf("3\r\n");
        wavhead->chunkSize = sectorsize * 1024 * 2 + 36; //Size of entire file -8;
        wavhead->subchunk2Size = sectorsize * 1024 * 2;  //Size of data
        res = f_lseek(f_rec, 0);
        if(res == FR_OK)
        {
            res = f_write(f_rec, (const void *)wavhead, sizeof(wave_header_t), &bw); //Rewrites header data
            if(res == FR_OK)
            {
                f_close(f_rec);
                iomem_free(wavhead);
                iomem_free(f_rec); 
                iomem_free(rx_buf);
                iomem_free(g_rx_dma_buf);
                sectorsize = 0;
            }
            else
            {
                printf("record file write failed!\r\n");
                f_close(f_rec);
                iomem_free(wavhead);
                iomem_free(f_rec);
                iomem_free(rx_buf);
                iomem_free(g_rx_dma_buf);
                sectorsize = 0;
                return;
            }
        } 
        else
        {
            printf("record file f_lseek failed! %d\r\n", res);
            f_close(f_rec);
            iomem_free(wavhead);
            iomem_free(f_rec);
            iomem_free(rx_buf);
            iomem_free(g_rx_dma_buf);
            sectorsize = 0;
            return;
        }                                                

        //printf("record finish!\r\n");
        //printf("file name:%s\r\n", path + 2); 
        //printf("chunkSize:%08x, subchunk2Size:%08x!\r\n", wavhead->chunkSize, wavhead->subchunk2Size);
        app_rgb_red_state(LIGHT_OFF); 
         
    }
    
    printf("record finish!\r\n");
    return;
}

/*External calls save the SD card */
void bsp_record_save_sd(void)
{
    // uint32_t bw;
    // uint8_t *temp = NULL;
    // if(g_recorder_switch == 1)
    // {
        
    //     if(queue_pop(recordQueue, temp) == 0)
    //     {
            
    //         int res = f_write(f_rec, temp, 1024 * 2, &bw); 
    //         printf("err:%d\r\n", res);

    //         if(res)
    //         {
    //             printf("err:%d\r\n", res);
    //             printf("bw:%d\r\n", bw);
    //             f_close(f_rec);
    //             iomem_free(wavhead);
    //             iomem_free(f_rec);
    //             bsp_recorder_deinit();
    //             return; 
    //         }
    //         //printf("1bw:%d\r\n", bw);
    //         sectorsize++; 
    //     }
        
    // }
    // msleep(5);

}