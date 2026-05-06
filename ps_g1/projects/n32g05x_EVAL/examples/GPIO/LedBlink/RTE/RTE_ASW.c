#include "RTE_ASW.h"

/**********************************************************************
*模块名称 : ASW 初始化
*描述      :
*编辑时间  :
*备注      : 初版
***********************************************************************/
void RTE_Call_ASW_Init(void)
{
    APP_Led_Init();
    APP_ChargeManagerInit(); // 应用层初始化
}

/**********************************************************************
*模块名称 : ASW主函数
*描述      :
*编辑时间  :
*备注      : 初版
***********************************************************************/
void RTE_Call_ASW_MainFunction_1ms(void)
{
    APP_Diag_ChargeManageMain_1ms(); // 充放电 漏电检测1ms任务
}

void RTE_Call_ASW_MainFunction_5ms(void)
{
    APP_ChargeManageMain();
}

void RTE_Call_ASW_MainFunction_10ms(void)
{
    Led_MainFunction();
}

void RTE_Call_ASW_MainFunction_1s(void)
{
    Log_Printf_Main();
}
