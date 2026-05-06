#ifndef _ADCM_H_
#define _ADCM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "AdcIf.h"
#include "AdcM_Cfg.h"
#include "AdcF.h"
#include "misc.h"

/****电压结构体****/
#define ADC_OFFSET               2048    // 直流偏置对应的ADC值（2.5V）
#define ADC_LIST_VOL_LEN         (140)   // 4.2K的采样率 采样10个点 2ms 进行一次判定
#define ADC_LIST_VOL_CAL_LEN     (120)
#define ADC_LIST_VOL_LOWER_ZEROLINE  (5)
#define ADC_LIST_VOL_EFFPOINT_H  (120)
#define ADC_LIST_VOL_EFFPOINT_L  (10)
#define ADC_LIST_VOL_ZEROPOINT_H (100)

typedef struct {
    uint8_t startFlag;    // 采样或计算标记
    uint8_t zeroPoint;    // 无效点计为0
    uint8_t effPoint;     // 有效点

    uint16_t adc;         // 采样值
    uint16_t adcList[ADC_LIST_VOL_LEN]; // 缓存列表
    uint16_t mulP;        // 系数

    uint32_t sum;         // 平方和
    uint32_t rmsVolX1000;
    uint32_t rmsVolX1000Last;
    uint32_t rmsVolFilterX1000;
    uint32_t rmsVolFilter2X1000;
    uint32_t calrmsVolX1000;
    uint32_t calrmsVolX1000Last;
    uint32_t calrmsFilterVolX1000;

    float rmsADC;
    float calK;
    float calD;
} ADCM_adcvol_info_t;

/****电流结构体****/
#define ADC_LIST_CUR_LEN         (140)
#define ADC_LIST_CUR_CAL_LEN     (120)
#define ADC_LIST_CUR_LOWER_ZEROLINE  (14)
#define ADC_LIST_CUR_EFFPOINT_H  (120)
#define ADC_LIST_CUR_EFFPOINT_L  (10)
#define ADC_LIST_CUR_ZEROPOINT_H (100)

typedef struct {
    uint8_t startFlag;    // 采样或计算标记
    uint16_t zeroPoint;   // 无效点计为0
    uint16_t effPoint;    // 有效点

    uint16_t adc;         // 采样值
    uint16_t adcmax;      // 最大采样值
    uint16_t maxAdcIndex;
    uint16_t maxAdcEffpointReg;
    uint16_t zeroContinusPoint;
    uint16_t conSamplePoints; // 连续采样点，防止0电流的时候 一直处于baseline的一侧
    uint16_t adcList[ADC_LIST_CUR_LEN]; // 缓存列表
    uint16_t mulP;        // 系数

    uint32_t sum;             // 平方和
    uint16_t adcmaxsum;       // 最大峰值四个数求和
    uint32_t rmsCurX1000;
    uint32_t rmsCuradcX1000;
    uint32_t rmsCurmaxX1000;  // 峰值计算结果
    uint32_t calrmsCurX1000;
    uint32_t calrmsCurX1000Last;
    uint32_t calrmsFilterCurX1000;

    float rmsADC;
    float calK;
    float calD;
} ADCM_adccur_info_t;

#define ADC_CP_FB_AMP_LIST_LEN      (80)
#define ADC_CP_FB_AMP_LIST_CAL_LEN  (70)
/****CP结构体****/
typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;

    uint16_t adc; // 采样值
    uint16_t adcList[ADC_CP_FB_AMP_LIST_LEN];
    uint16_t mulP; // 系数

    uint32_t sum;
    float adc_average;
} ADCM_adcp_info_t;

/****NTC1结构体****/
#define ADC_TEMP_NTC1_LIST_LEN     (80)
#define ADC_TEMP_NTC1_LIST_CAL_LEN (70)

// 定义温度-阻值对照表结构体
typedef struct {
    uint32_t voltage;       // 输入电压
    int temp;               // 对应温度（℃）
} TempVolTable;

typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;
    uint16_t adc; // 采样值
    uint16_t adcList[ADC_TEMP_NTC1_LIST_LEN];
    uint32_t sum;
    float adc_average;
    TempVolTable temp;      // 对应温度（℃）
} ADCM_adcntc1_info_t;

/****PGD结构体****/
#define ADC_PGD_LEN               (200)
#define ADC_PGD_Threadhold_LEN     (10)
#define AdcM_PGD_STATUS_OK        (0)
#define AdcM_PGD_STATUS_ERR       (1)

typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;
    uint16_t adc; // 采样值
    uint16_t adcList[ADC_PGD_LEN];
} ADCM_adcpgd_info_t;

/****EVDIO结构体****/
#define ADC_EVDIODE_CHK_LEN       (80)
#define AdcM_EVDIODE_STATUS_OK    (0)
#define AdcM_EVDIODE_STATUS_ERR   (1)

typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;
    uint16_t adc; // 采样值
    uint16_t adcList[ADC_EVDIODE_CHK_LEN];
    uint32_t sum;
    float adc_average;
    uint32_t val;
} ADCM_adcevdio_info_t;

/****RLYCHK结构体****/
#define ADC_RLY_CHK_MCU_LEN           (200)
#define ADC_RLY_CHK_Threadhold_LEN     (10)
#define AdcM_RLY_CHK_STATUS_CONNECT   (1)
#define AdcM_RLY_CHK_STATUS_BREAK     (0)

typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;
    uint16_t adc; // 采样值
    uint16_t adcList[ADC_RLY_CHK_MCU_LEN];
    // uint16_t mulP; // 系数
} ADCM_adcrlychk_info_t;

/****MCU TEMP结构体****/
/*MCU温度计算公式需求*/
#define AVG_SLOPE    0.0039f
#define TS_VALUE_ADDR    (0x1FFFF930U)
#define VDDA_5V    1
#ifdef VDDA_5V
#define VREF_VALUE      (5.0)   //this value need be configured by the actual voltage reference.
#define VTS_CODE_ADDR   (0x1FFFF9D8U)
#else
#define VREF_VALUE      (3.3)
#define VTS_CODE_ADDR   (0x1FFFF928U)
#endif
#define ADC_MCU_Temp_LEN       (50)

typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;
    uint16_t adc; // 采样值
    uint16_t adcList[ADC_MCU_Temp_LEN];
    uint32_t sum;
    float adc_average;
} ADCM_adcmcutemp_info_t;

/****shortcircuit结构体****/
#define ADC_SHORTCIRCUIT_LEN         (50)
#define ADC_SHORTCIRCUIT_Threadhold_LEN (10)
#define AdcM_SHORTCIRCUIT_STATUS_OK  (0)
#define AdcM_SHORTCIRCUIT_STATUS_ERR  (1)

typedef struct {
    uint8_t startFlag;
    uint8_t zeroPoint;
    uint8_t effPoint;

    uint16_t adc; // 采样值
    uint16_t adcList[ADC_SHORTCIRCUIT_LEN];
} ADCM_adcshortcircuit_info_t;

/****ADC原始采样值结构体****/
typedef struct {
    ADCM_adcvol_info_t voltage;
    ADCM_adccur_info_t current;
    ADCM_adcp_info_t cp;
    ADCM_adcntc1_info_t ntc1;
    ADCM_adcpgd_info_t ground;
    ADCM_adcevdio_info_t evdio;
    ADCM_adcrlychk_info_t rlychk;
    ADCM_adcmcutemp_info_t mcutemp;
    ADCM_adcshortcircuit_info_t shortcircuit;
} ADCM_adcoriginal_t; // ADC原始采样值

/****ADC-debug结构体****/
typedef struct {
    uint16_t flag;
    uint32_t Value;
} ADCM_debug_voltage_info_t;

typedef struct {
    uint16_t flag;
    uint32_t Value;
} ADCM_debug_current_info_t;

typedef struct {
    uint16_t flag;
    uint32_t Value;
} ADCM_debug_cp_info_t;

typedef struct {
    uint16_t flag;
    uint32_t pgdstatus;
} ADCM_debug_pgd_info_t;

typedef struct {
    uint16_t flag;
    uint32_t Value;
} ADCM_debug_evdio_info_t;

typedef struct {
    uint16_t flag;
    uint32_t rlychkstatus;
} ADCM_debug_rlychk_info_t;

typedef struct {
    uint16_t flag;
    int32_t Value;
} ADCM_debug_mcutemp_info_t;

typedef struct {
    uint16_t flag;
    uint32_t Value;
} ADCM_debug_ntc1vol_info_t;

typedef struct {
    uint16_t flag;
    int32_t Value;
} ADCM_debug_ntc1temp_info_t;

typedef struct {
    uint16_t flag;
    uint32_t leakagestatus;
} ADCM_debug_leakage_info_t;

typedef struct {
    uint16_t flag;
    uint32_t shortcircuitstatus;
} ADCM_debug_shortcircuit_info_t;

/****ADC-cali结构体****/
typedef struct {
    uint32_t buff[2];
    int32_t cail_k;   /* [NOTE] 改为int32_t以支持负数，二进制与uint32_t相同，flash读写不受影响 */
    int32_t cail_b;   /* [NOTE] 同上 */
} ADCM_cali_voltage_info_t;

typedef struct {
    // uint16_t flag;
    uint32_t buff[2];
    int32_t cail_k;
    int32_t cail_b;
} ADCM_cali_current_info_t;

/****ADC监控器结构体****/
typedef struct {
    ADCM_adcoriginal_t adcoriginal;
} ADCM_monitor_info_t;

/****ADCdebug结构体****/
typedef struct {
    ADCM_debug_voltage_info_t voltage;
    ADCM_debug_current_info_t current;
    ADCM_debug_cp_info_t cp;
    ADCM_debug_ntc1vol_info_t ntc1vol;
    ADCM_debug_ntc1temp_info_t ntc1temp;
    ADCM_debug_pgd_info_t ground;
    ADCM_debug_evdio_info_t evdio;
    ADCM_debug_rlychk_info_t rlychk;
    ADCM_debug_mcutemp_info_t mcutemp;
    ADCM_debug_leakage_info_t leakage;
    ADCM_debug_shortcircuit_info_t shortcircuit;
} ADCM_debug_info_t;

/****ADCcali结构体****/
typedef struct {
    ADCM_cali_voltage_info_t voltage;
    ADCM_cali_current_info_t current;
} ADCM_cali_info_t;

/****ADCM总结构体****/
typedef struct {
    ADCM_monitor_info_t monitor;
    ADCM_debug_info_t debug;
    ADCM_cali_info_t cali;
} ADCM_info_t;

extern ADCM_info_t ADCM_t;

/****ADCM外部调用结果结构体****/
typedef struct {
    uint32_t AdcM_Voltage_Val;
    uint32_t AdcM_Current_Val;
    uint32_t AdcM_CP_Val;
    uint32_t AdcM_NTC1_Val;
    int32_t AdcM_NTC1Temp_Val;
    uint32_t AdcM_Ground_Sts;
    uint32_t AdcM_EvdiodeCHK_Sts;
    uint32_t AdcM_RelayCHK_Sts;
    int32_t AdcM_MCUTemp_Val;
    uint32_t AdcM_shortcircuit_Sts;
    uint32_t AdcM_VOL_RMSx1000;
    uint32_t AdcM_CUR_RMSx1000;
} ADC_Val_t;

/* 函数声明 */
extern void AdcM_CallbackSampleValue(void);
extern int32_t AdcM_GetValidValue(AdcCfg_ChanEnumType ucChannal);

extern void AdcM_get_Voltage_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_Current_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_CP_update(ADCM_info_t *ADCM,uint16_t Samp_Val);
extern void AdcM_get_NTC1Temp_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_Ground_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_EvdiodeCHK_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_RelayCHK_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_MCUTemp_update(ADCM_info_t *ADCM, uint16_t Samp_Val);
extern void AdcM_get_ShortCircuit_update(ADCM_info_t *ADCM, uint16_t Samp_Val);

#endif /* _ADCM_H_ */








