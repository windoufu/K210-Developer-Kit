#include "angle.h"

static quaternion_t NumQ = {1, 0, 0, 0};
float vecxZ, vecyZ, veczZ;
float wz_acc_tmp[2];

attitude_t g_attitude;
icm_data_t g_icm20607;


/* Four elements integration angle */
static void get_angle(attitude_t *p_angle)
{
    vecxZ = 2 * NumQ.q0 * NumQ.q2 - 2 * NumQ.q1 * NumQ.q3; /*Matrix (3,1) item*/                                 
    vecyZ = 2 * NumQ.q2 * NumQ.q3 + 2 * NumQ.q0 * NumQ.q1; /*Matrix (3,2) item*/                                 
    veczZ = NumQ.q0 * NumQ.q0 - NumQ.q1 * NumQ.q1 - NumQ.q2 * NumQ.q2 + NumQ.q3 * NumQ.q3; /*Matrix (3,3) item*/  

    p_angle->pitch = asin(vecxZ) * RtA;             //Pitch angle
    p_angle->roll = atan2f(vecyZ, veczZ) * RtA;     //Roll angle
}

/* Reset the four elements */
void reset_quaternion(void)
{
    NumQ.q0 = 1.0;
    NumQ.q1 = 0.0;
    NumQ.q2 = 0.0;
    NumQ.q3 = 0.0;
}

/* Four elements acquisition dt: about 5MS */
void get_attitude_angle(icm_data_t *p_icm, attitude_t *p_angle, float dt)
{
    vector_t Gravity, Acc, Gyro, AccGravity;
    static vector_t GyroIntegError = {0};
    static float KpDef = 0.8f;
    static float KiDef = 0.0003f;
    float q0_t, q1_t, q2_t, q3_t;
    float NormQuat;
    float HalfTime = dt * 0.5f;

    Gravity.x = 2 * (NumQ.q1 * NumQ.q3 - NumQ.q0 * NumQ.q2);
    Gravity.y = 2 * (NumQ.q0 * NumQ.q1 + NumQ.q2 * NumQ.q3);
    Gravity.z = 1 - 2 * (NumQ.q1 * NumQ.q1 + NumQ.q2 * NumQ.q2);
    // Normalized acceleration,
    NormQuat = q_rsqrt(squa(p_icm->accX)+ squa(p_icm->accY) +squa(p_icm->accZ)); 

    //After normalization, it can be reduced to the downward direction component of the unit vector
    Acc.x = p_icm->accX * NormQuat;
    Acc.y = p_icm->accY * NormQuat;
    Acc.z = p_icm->accZ * NormQuat;

    //The value obtained by the cross product of the vector. After the cross product, the deviation of the gravity component of the rotation matrix on the new acceleration component can be obtained
    AccGravity.x = (Acc.y * Gravity.z - Acc.z * Gravity.y);
    AccGravity.y = (Acc.z * Gravity.x - Acc.x * Gravity.z);
    AccGravity.z = (Acc.x * Gravity.y - Acc.y * Gravity.x);

    GyroIntegError.x += AccGravity.x * KiDef;
    GyroIntegError.y += AccGravity.y * KiDef;
    GyroIntegError.z += AccGravity.z * KiDef;

    //Angular velocity fusion acceleration proportional compensation value, and the above three sentences together form PI compensation, and get the corrected angular velocity value
    Gyro.x = p_icm->gyroX * Gyro_Gr + KpDef * AccGravity.x + GyroIntegError.x; //弧度制，此处补偿的是角速度的漂移
    Gyro.y = p_icm->gyroY * Gyro_Gr + KpDef * AccGravity.y + GyroIntegError.y;
    Gyro.z = p_icm->gyroZ * Gyro_Gr + KpDef * AccGravity.z + GyroIntegError.z;

    q0_t = (-NumQ.q1 * Gyro.x - NumQ.q2 * Gyro.y - NumQ.q3 * Gyro.z) * HalfTime;
    q1_t = (NumQ.q0 * Gyro.x - NumQ.q3 * Gyro.y + NumQ.q2 * Gyro.z) * HalfTime;
    q2_t = (NumQ.q3 * Gyro.x + NumQ.q0 * Gyro.y - NumQ.q1 * Gyro.z) * HalfTime;
    q3_t = (-NumQ.q2 * Gyro.x + NumQ.q1 * Gyro.y + NumQ.q0 * Gyro.z) * HalfTime;

    NumQ.q0 += q0_t; 
    NumQ.q1 += q1_t;
    NumQ.q2 += q2_t;
    NumQ.q3 += q3_t;

    NormQuat = q_rsqrt(squa(NumQ.q0) + squa(NumQ.q1) + squa(NumQ.q2) + squa(NumQ.q3));
    NumQ.q0 *= NormQuat;                                                               
    NumQ.q1 *= NormQuat;
    NumQ.q2 *= NormQuat;
    NumQ.q3 *= NormQuat;

    get_angle(p_angle);
}
