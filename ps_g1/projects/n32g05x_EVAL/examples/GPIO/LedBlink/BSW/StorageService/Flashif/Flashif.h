#ifndef __FLASHIF_H__
#define __FLASHIF_H__

#include "n32g05x.h"
#include "Flashif_Cfg.h"
#include "n32g05x_flash.h"
#include "misc.h"
#include "AdcM.h"
#include "cp.h"
#include "rtthread.h"

#define CALIBRATION_VOL_A_ADDR    0x1FFF1000 // 校准电压参数存储地址 A区
#define CALIBRATION_VOL_B_ADDR    0x1FFF1200 // 校准电压参数存储地址 B区
#define CALIBRATION_CUR_A_ADDR    0x1FFF1400 // 校准电流参数存储地址 A区
#define CALIBRATION_CUR_B_ADDR    0x1FFF1600 // 校准电流参数存储地址 B区
#define CALIBRATION_CRC_ADDR      0x1FFF1800 // 校准参数CRC存储地址

#define FACTORY_INFO_ADDR 0x1FFF2000 // 工厂模式标志存储地址

/* 函数声明 */
extern void FlsIf_ReadMultiple(uint32_t address, uint32_t* buf, uint32_t len);
extern void FlsIf_ReadPage(uint32_t address, uint32_t* buf);
extern void FlsIf_ProgramMultiple(uint32_t address, const uint32_t* pBuffer, uint32_t len);
extern void FlsIf_ProgramPage(uint32_t address, const uint32_t* pBuffer);
extern void FlsIf_ErasePage(uint32_t pageAddr);
extern uint32_t calibration_crc32(const uint32_t *data, uint32_t length);
extern void FlsIf_Version_Init(void);
extern void FlsIf_cali_voltage_Init(void);
extern void FlsIf_cali_current_Init(void);
extern void FlsIf_Init(void);

#endif /* __FLASHIF_H__ */
