#include "n32g05x.h"
#include "AdcM.h"
#include <math.h>
#include <string.h>
#include "log.h"
#include "RTE_BSW.h"
//#include "AdcF.h"
ADCM_info_t ADCM_t;
ADC_Val_t AdcM_ADC_Val;

// 温度查表
// 格式: [电压×100, 温度℃]
// 电压单调递减, 温度单调递增
TempVolTable table1[] = {
    {4991, -40}, {4988, -35}, {4984, -30}, {4979, -25}, {4972, -20},
    {4963, -15}, {4952, -10}, {4938, -5}, {4920, 0}, {4899, 5},
    {4874, 10}, {4842, 15}, {4803, 20}, {4757, 25}, {4702, 30},
    {4637, 35}, {4562, 40}, {4476, 45}, {4377, 50}, {4266, 55},
    {4143, 60}, {4006, 65}, {3859, 70}, {3702, 75}, {3536, 80},
    {3363, 85}, {3183, 90}, {3001, 95}, {2819, 100}, {2636, 105},
    {2455, 110},
};
#define TABLE1_SZ  (sizeof(table1)/sizeof(table1[0]))

TempVolTable table2[] = {
    {1729, -40}, {1728, -35}, {1727, -30}, {1726, -25}, {1725, -20},
    {1724, -15}, {1723, -10}, {1722, -5}, {1721,  0}, {1718,  5},
    {1715, 10}, {1711, 15}, {1706, 20}, {1700, 25}, {1693, 30},
    {1685, 35}, {1675, 40}, {1663, 45}, {1649, 50}, {1635, 55},
    {1615, 60}, {1594, 65}, {1570, 70}, {1543, 75}, {1513, 80},
    {1481, 85}, {1445, 90}, {1406, 95}, {1365, 100}, {1320, 105},
    {1273, 110},
};
#define TABLE2_SZ  (sizeof(table2)/sizeof(table2[0]))

/**
*\name    AdcM_get_Voltage_update.
*\fun     采样 Voltage
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_Voltage_update(ADCM_info_t *ADCM,uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.voltage.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.voltage.startFlag == 1) /* =1 开始采样*/
    {
        if (ADCM->monitor.adcoriginal.voltage.adc < ADC_OFFSET)  // <2048
        {
            ADCM->monitor.adcoriginal.voltage.zeroPoint++;
            if (ADCM->monitor.adcoriginal.voltage.zeroPoint > ADC_LIST_VOL_LOWER_ZEROLINE) //>5为有效点
            {
                ADCM->monitor.adcoriginal.voltage.startFlag = 2;  //标志位置2
            }
        }
        else
        {
            ADCM->monitor.adcoriginal.voltage.zeroPoint = 0; // >= 2048  清空计数
        }
    }
    if (ADCM->monitor.adcoriginal.voltage.startFlag == 2)
    {
        if (ADCM->monitor.adcoriginal.voltage.adc > ADC_OFFSET)
        {
            ADCM->monitor.adcoriginal.voltage.startFlag = 3;//标志位 置3
            ADCM->monitor.adcoriginal.voltage.zeroPoint = 0;
        }
    }
    if (ADCM->monitor.adcoriginal.voltage.startFlag == 3)
    {
        if (ADCM->monitor.adcoriginal.voltage.adc < ADC_OFFSET)
        {
            ADCM->monitor.adcoriginal.voltage.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.voltage.adcList[ADCM->monitor.adcoriginal.voltage.effPoint] = ADCM->monitor.adcoriginal.voltage.adc;
            ADCM->monitor.adcoriginal.voltage.effPoint++;
        }
        // 如果采集的点超过了 则清0
        if ((ADCM->monitor.adcoriginal.voltage.zeroPoint + ADCM->monitor.adcoriginal.voltage.effPoint) >= ADC_LIST_VOL_CAL_LEN)
        {
            ADCM->monitor.adcoriginal.voltage.startFlag = 0;
        }
        // 如果采集的点有n个低于baseline 则认为可以进行计算
        if (ADCM->monitor.adcoriginal.voltage.zeroPoint > 5)
        {
            ADCM->monitor.adcoriginal.voltage.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_Voltage_update.
*\fun     计算 Voltage
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_Voltage_update(ADCM_info_t *ADCM)
{
    uint16_t eValue = 0;
    ADCM->monitor.adcoriginal.voltage.calK = (float)ADCM_t.cali.voltage.cail_k / 1000.0f;
    ADCM->monitor.adcoriginal.voltage.calD = (float)ADCM_t.cali.voltage.cail_b / 10.0f;
    if (ADCM->monitor.adcoriginal.voltage.startFlag == 0) // ==0 表示开启计算
    {
        if((ADCM->monitor.adcoriginal.voltage.adcList[ADCM->monitor.adcoriginal.voltage.effPoint-1]-2048)>15)
        {
            ADCM->monitor.adcoriginal.voltage.adcList[ADCM->monitor.adcoriginal.voltage.effPoint] = ADCM->monitor.adcoriginal.voltage.adcList[ADCM->monitor.adcoriginal.voltage.effPoint-1] - 15;
            ADCM->monitor.adcoriginal.voltage.effPoint++;
        }
        if((ADCM->monitor.adcoriginal.voltage.adcList[0]-2048)>30)
        {
            ADCM->monitor.adcoriginal.voltage.adcList[ADCM->monitor.adcoriginal.voltage.effPoint] = ADCM->monitor.adcoriginal.voltage.adcList[ADCM->monitor.adcoriginal.voltage.effPoint-1] - 15;
            ADCM->monitor.adcoriginal.voltage.effPoint++;
        }
        for (int i = 0; i < ADCM->monitor.adcoriginal.voltage.effPoint; i++)
        {
            eValue = ADCM->monitor.adcoriginal.voltage.adcList[i] - ADC_OFFSET;
            ADCM->monitor.adcoriginal.voltage.sum += eValue * eValue;
        }
        // 此处不应该简单的判定effpoint的, 这里合法的effpoint 几乎是确定的, 要有一个范围
        if ((ADCM->monitor.adcoriginal.voltage.effPoint > ADC_LIST_VOL_EFFPOINT_L) &&
            (ADCM->monitor.adcoriginal.voltage.effPoint < ADC_LIST_VOL_EFFPOINT_H))
        {
            // 此处是合法的数据, 需要进行数据更新
            ADCM->monitor.adcoriginal.voltage.rmsADC = hard_root(ADCM->monitor.adcoriginal.voltage.sum / ADCM->monitor.adcoriginal.voltage.effPoint);
            ADCM->monitor.adcoriginal.voltage.rmsVolX1000 = ADCM->monitor.adcoriginal.voltage.rmsADC * 675;

            if (ADCM->debug.voltage.flag) // 串口校准标记
            {
                AdcM_ADC_Val.AdcM_Voltage_Val = ADCM->debug.voltage.Value;//串口进来电压值给计算结果
            }

            if (threadHold_check_u32(ADCM->monitor.adcoriginal.voltage.rmsVolX1000, ADCM->monitor.adcoriginal.voltage.rmsVolX1000Last, 40000) == 1) // 变化超过40V//20250703-001-003
            {
                ADCM->monitor.adcoriginal.voltage.rmsVolFilterX1000 = ADCM->monitor.adcoriginal.voltage.rmsVolX1000;
                ADCM->monitor.adcoriginal.voltage.rmsVolFilter2X1000 = ADCM->monitor.adcoriginal.voltage.rmsVolX1000;
            }
            else // 变化低于10V
            {
                ADCM->monitor.adcoriginal.voltage.rmsVolFilterX1000 =
                ADCM->monitor.adcoriginal.voltage.rmsVolFilterX1000 * 0.6 + ADCM->monitor.adcoriginal.voltage.rmsVolX1000 * 0.4; // 20250706-001-001

                ADCM->monitor.adcoriginal.voltage.rmsVolFilter2X1000 =
                ADCM->monitor.adcoriginal.voltage.rmsVolFilter2X1000 * 0.6 + ADCM->monitor.adcoriginal.voltage.rmsVolFilterX1000 * 0.4;
            }
            ADCM->monitor.adcoriginal.voltage.rmsVolX1000Last = ADCM->monitor.adcoriginal.voltage.rmsVolX1000;
            ADCM->monitor.adcoriginal.voltage.calrmsVolX1000 = ADCM->monitor.adcoriginal.voltage.rmsVolFilter2X1000 * ADCM->monitor.adcoriginal.voltage.calK + ADCM->monitor.adcoriginal.voltage.calD;
            if (threadHold_check_u32(ADCM->monitor.adcoriginal.voltage.calrmsVolX1000, ADCM->monitor.adcoriginal.voltage.calrmsVolX1000Last, 10000) == 1) // 变化超过10V//20250703-001-003
            {
                AdcM_ADC_Val.AdcM_Voltage_Val = (uint32_t)ADCM->monitor.adcoriginal.voltage.calrmsVolX1000;
                AdcM_ADC_Val.AdcM_VOL_RMSx1000 = (uint32_t)ADCM->monitor.adcoriginal.voltage.rmsVolFilter2X1000; //校准打印不带校准参数电压
            }
            else // 变化低于10V
            {
                AdcM_ADC_Val.AdcM_Voltage_Val = (uint32_t) (ADCM->monitor.adcoriginal.voltage.calrmsVolX1000 * 0.1 + AdcM_ADC_Val.AdcM_Voltage_Val * 0.9);
                AdcM_ADC_Val.AdcM_VOL_RMSx1000 = (uint32_t) (ADCM->monitor.adcoriginal.voltage.rmsVolFilter2X1000 * 0.1 + AdcM_ADC_Val.AdcM_VOL_RMSx1000 * 0.9); //校准打印不带校准参数电压
            }
            ADCM->monitor.adcoriginal.voltage.calrmsVolX1000Last = ADCM->monitor.adcoriginal.voltage.calrmsVolX1000;
        }
        else
        {
            // 此处是非法数据, 需要丢弃, 并保持原来的数据
        }
        if (ADCM->monitor.adcoriginal.voltage.zeroPoint > ADC_LIST_VOL_ZEROPOINT_H) // 一共最多120个点进行计算 超过100个点低于baseline则认为0V
        {
            ADCM->monitor.adcoriginal.voltage.rmsADC = 0;
            ADCM->monitor.adcoriginal.voltage.rmsVolX1000 = 0;
            ADCM->monitor.adcoriginal.voltage.calrmsVolX1000 = 0;
            AdcM_ADC_Val.AdcM_Voltage_Val = 0;
            AdcM_ADC_Val.AdcM_VOL_RMSx1000 = 0; //校准打印不带校准参数电压
        }
        ADCM->monitor.adcoriginal.voltage.effPoint = 0;
        ADCM->monitor.adcoriginal.voltage.zeroPoint = 0;
        ADCM->monitor.adcoriginal.voltage.sum = 0;
        memset(ADCM->monitor.adcoriginal.voltage.adcList, 0, sizeof(ADCM->monitor.adcoriginal.voltage.adcList));
        ADCM->monitor.adcoriginal.voltage.startFlag = 1;
    }
}

/**
*\name    AdcM_get_Current_update.
*\fun     采样 Current
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_Current_update(ADCM_info_t *ADCM,uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.current.adc = Samp_Val;
    if (ADCM->monitor.adcoriginal.current.startFlag == 1) // ==1 表示开启监测
    {
        if ( ADCM->monitor.adcoriginal.current.adc < 10 ) //co->ippc.monitor.adcOri.cur.baseLine = 10
        {
            ADCM->monitor.adcoriginal.current.zeroPoint++;
            if (ADCM->monitor.adcoriginal.current.zeroPoint > ADC_LIST_CUR_LOWER_ZEROLINE)
            {
                ADCM->monitor.adcoriginal.current.startFlag = 2;
            }
        }
        else
        {
            ADCM->monitor.adcoriginal.current.zeroPoint = 0;
            if (ADCM->monitor.adcoriginal.current.conSamplePoints < ADC_LIST_CUR_LEN)
            {
                ADCM->monitor.adcoriginal.current.conSamplePoints++;
            }
            else
            {
                ADCM->monitor.adcoriginal.current.startFlag = 2;
            }
        }
        ADCM->monitor.adcoriginal.current.zeroContinusPoint = 0;
    }
    else
    {
        ADCM->monitor.adcoriginal.current.conSamplePoints = 0;
    }
    if (ADCM->monitor.adcoriginal.current.startFlag == 2)
    {
        if ( ADCM->monitor.adcoriginal.current.adc > 10 ) //co->ippc.monitor.adcOri.cur.baseLine = 10
        {
            ADCM->monitor.adcoriginal.current.startFlag = 3;
            ADCM->monitor.adcoriginal.current.zeroPoint = 0;
        }
        else
        {
            if (ADCM->monitor.adcoriginal.current.zeroContinusPoint < ADC_LIST_CUR_LEN)
            {
                ADCM->monitor.adcoriginal.current.zeroContinusPoint++;
            }
            else
            {
                ADCM->monitor.adcoriginal.current.startFlag = 3;
            }
        }
    }
    if (ADCM->monitor.adcoriginal.current.startFlag == 3)
    {
        if ( ADCM->monitor.adcoriginal.current.adc < 10 ) //co->ippc.monitor.adcOri.cur.baseLine = 10
        {
            ADCM->monitor.adcoriginal.current.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.current.adcList[ADCM->monitor.adcoriginal.current.effPoint] = ADCM->monitor.adcoriginal.current.adc;
            ADCM->monitor.adcoriginal.current.effPoint++;
        }
        if ((ADCM->monitor.adcoriginal.current.zeroPoint + ADCM->monitor.adcoriginal.current.effPoint) > ADC_LIST_CUR_CAL_LEN)
        {
            ADCM->monitor.adcoriginal.current.startFlag = 0;
        }
        // 如果采集的点有n个低于baseline 则认为可以进行计算
        if (ADCM->monitor.adcoriginal.current.zeroPoint > ADC_LIST_CUR_LOWER_ZEROLINE)
        {
            ADCM->monitor.adcoriginal.current.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_Current_update.
*\fun     计算 Current
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_Current_update(ADCM_info_t *ADCM)
{
    uint16_t temp[5];
    uint16_t eValue = 0;
    ADCM->monitor.adcoriginal.current.calK = (float)ADCM_t.cali.current.cail_k/1000;
    ADCM->monitor.adcoriginal.current.calD = (float)ADCM_t.cali.current.cail_b/10;
    ADCM->monitor.adcoriginal.current.adcmax = 0;
    ADCM->monitor.adcoriginal.current.adcmaxsum = 0;
    if(ADCM->monitor.adcoriginal.current.startFlag == 0) // ==1 表示开启监测
    {
        for(int i = 0; i < ADCM->monitor.adcoriginal.current.effPoint; i++)
        {
            eValue = ADCM->monitor.adcoriginal.current.adcList[i] - 10;
            ADCM->monitor.adcoriginal.current.sum += eValue * eValue;
        }
        // 1. 从大到小排序
        sortDesc(ADCM->monitor.adcoriginal.current.adcList, sizeof(ADCM->monitor.adcoriginal.current.adcList) / sizeof(ADCM->monitor.adcoriginal.current.adcList[0]));
        // 2. 取出最大的5个数
        for (int i = 0; i < 5; i++) {
            temp[i] = ADCM->monitor.adcoriginal.current.adcList[i];
        }
        // 3. 舍弃最大值（第一个）, 剩下4个求和
        for (int i = 1; i < 5; i++) {
            ADCM->monitor.adcoriginal.current.adcmaxsum += temp[i];
        }
        ADCM->monitor.adcoriginal.current.adcmax = ADCM->monitor.adcoriginal.current.adcmaxsum/4;
        // 此处不应该简单的判定effpoint的, 这里合法的effpoint 几乎是确定的, 要有一个范围
        if((ADCM->monitor.adcoriginal.current.effPoint > ADC_LIST_CUR_EFFPOINT_L) &&
            (ADCM->monitor.adcoriginal.current.effPoint < ADC_LIST_CUR_EFFPOINT_H))
        {
            // 此处是合法的数据, 需要进行数据更新
            // 计算平均值并开平方, 得到电流的有效值(RMS)
            ADCM->monitor.adcoriginal.current.rmsADC = hard_root(ADCM->monitor.adcoriginal.current.sum / ADCM->monitor.adcoriginal.current.effPoint);
            ADCM->monitor.adcoriginal.current.rmsCuradcX1000 = ADCM->monitor.adcoriginal.current.rmsADC * 478.4f/10.0f;// 针对电阻51欧 // co->ippc.monitor.adcOri.cur.mulP = 244;
            ADCM->monitor.adcoriginal.current.rmsCurmaxX1000 = ADCM->monitor.adcoriginal.current.adcmax/sqrt(2) * 478.4/10;
            if(ADCM->monitor.adcoriginal.current.rmsCuradcX1000 < 28000)
            {
                ADCM->monitor.adcoriginal.current.rmsCurX1000 = ADCM->monitor.adcoriginal.current.rmsCuradcX1000;
            }
            else
            {
                ADCM->monitor.adcoriginal.current.rmsCurX1000 = ADCM->monitor.adcoriginal.current.rmsCurmaxX1000;
            }

            if(ADCM->debug.current.flag) // 串口控制
            {
                ADCM->monitor.adcoriginal.current.rmsCurX1000 = ADCM->debug.current.Value;
            }
            if(ADCM->monitor.adcoriginal.current.calD < 0)
            {
                if(ADCM->monitor.adcoriginal.current.rmsCurX1000 * ADCM->monitor.adcoriginal.current.calK > ADCM->monitor.adcoriginal.current.calD)
                {
                    ADCM->monitor.adcoriginal.current.calrmsCurX1000 =
                    ADCM->monitor.adcoriginal.current.rmsCurX1000 * ADCM->monitor.adcoriginal.current.calK + ADCM->monitor.adcoriginal.current.calD;
                }
                else
                {
                    ADCM->monitor.adcoriginal.current.calrmsCurX1000 = 0;
                }
            }
            else
            {
                ADCM->monitor.adcoriginal.current.calrmsCurX1000 =
                ADCM->monitor.adcoriginal.current.rmsCurX1000 * ADCM->monitor.adcoriginal.current.calK + ADCM->monitor.adcoriginal.current.calD;
            }
            AdcM_ADC_Val.AdcM_Current_Val = (uint32_t)(AdcM_ADC_Val.AdcM_Current_Val * 0.5 + ADCM->monitor.adcoriginal.current.calrmsCurX1000 * 0.5);
            AdcM_ADC_Val.AdcM_CUR_RMSx1000 = (uint32_t)(AdcM_ADC_Val.AdcM_CUR_RMSx1000 * 0.5 + ADCM->monitor.adcoriginal.current.rmsCurX1000 * 0.5); //校准打印不带校准参数的电流值
        }
        else
        {
            ADCM->monitor.adcoriginal.current.rmsADC = 0;
            ADCM->monitor.adcoriginal.current.rmsCurX1000 = 0;
            if(ADCM->debug.current.flag) //串口控制
            {
                ADCM->monitor.adcoriginal.current.rmsCurX1000 = ADCM->debug.current.Value;
            }
            if(ADCM->monitor.adcoriginal.current.calD < 0)
            {
                if(ADCM->monitor.adcoriginal.current.rmsCurX1000 * ADCM->monitor.adcoriginal.current.calK > ADCM->monitor.adcoriginal.current.calD)
                {
                    ADCM->monitor.adcoriginal.current.calrmsCurX1000 =
                    ADCM->monitor.adcoriginal.current.rmsCurX1000 * ADCM->monitor.adcoriginal.current.calK + ADCM->monitor.adcoriginal.current.calD;
                }
                else
                {
                    ADCM->monitor.adcoriginal.current.calrmsCurX1000 = 0;
                }
            }
            else
            {
                ADCM->monitor.adcoriginal.current.calrmsCurX1000 =
                ADCM->monitor.adcoriginal.current.rmsCurX1000 * ADCM_t.cali.current.cail_k + ADCM->monitor.adcoriginal.current.calD;
            }
        
        if(threadHold_check_u32(ADCM->monitor.adcoriginal.current.calrmsCurX1000, ADCM->monitor.adcoriginal.current.calrmsCurX1000Last, 2000) == 1) // 变化超过10V
        {
            AdcM_ADC_Val.AdcM_Current_Val = (uint32_t)ADCM->monitor.adcoriginal.current.calrmsCurX1000;
            AdcM_ADC_Val.AdcM_CUR_RMSx1000 = (uint32_t)ADCM->monitor.adcoriginal.current.rmsCurX1000; //校准打印不带校准参数的电流值
        }
        else // 变化低于2A
        {
            AdcM_ADC_Val.AdcM_Current_Val = (uint32_t)(AdcM_ADC_Val.AdcM_Current_Val * 0.5 + ADCM->monitor.adcoriginal.current.calrmsCurX1000 * 0.5);
            AdcM_ADC_Val.AdcM_CUR_RMSx1000 = (uint32_t)(AdcM_ADC_Val.AdcM_CUR_RMSx1000 * 0.5 + ADCM->monitor.adcoriginal.current.rmsCurX1000 * 0.5);
        }

		ADCM->monitor.adcoriginal.current.calrmsCurX1000Last = ADCM->monitor.adcoriginal.current.calrmsCurX1000;
	}

	
if(ADCM->monitor.adcoriginal.current.zeroPoint > ADC_LIST_CUR_ZEROPOINT_H) // 一共最多120个点进行计算 超过100个点低于baseline则认为0V
{
    if(!ADCM->debug.current.flag)
    {
        ADCM->monitor.adcoriginal.current.rmsADC = 0;
        ADCM->monitor.adcoriginal.current.rmsCuradcX1000 = 0;
        ADCM->monitor.adcoriginal.current.rmsCurmaxX1000 = 0;
        ADCM->monitor.adcoriginal.current.rmsCurX1000 = 0;
        ADCM->monitor.adcoriginal.current.calrmsCurX1000 = 0;
        AdcM_ADC_Val.AdcM_Current_Val = 0;
        AdcM_ADC_Val.AdcM_CUR_RMSx1000 = 0;
    }
}
	ADCM->monitor.adcoriginal.current.effPoint = 0;
	ADCM->monitor.adcoriginal.current.zeroPoint = 0;
	ADCM->monitor.adcoriginal.current.sum = 0;
	memset(ADCM->monitor.adcoriginal.current.adcList, 0, sizeof(ADCM->monitor.adcoriginal.current.adcList));
	ADCM->monitor.adcoriginal.current.startFlag = 1;
  }
}
/**
*\name    AdcM_get_CP_update.
*\fun     采样 CP
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_CP_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.cp.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.cp.startFlag == 1) /* =1 开始监测*/
    {
        //zeroLine = 400; //(约CP实际1.9V 400/4095 *5*5.3/2 +CP_SHIFT = 1.89V)
        if (ADCM->monitor.adcoriginal.cp.adc < 400) //(400*(5/4095))*((3.3+2)/2) = 1.29V 合并
        {
            ADCM->monitor.adcoriginal.cp.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.cp.adcList[ADCM->monitor.adcoriginal.cp.effPoint] = ADCM->monitor.adcoriginal.cp.adc;
            ADCM->monitor.adcoriginal.cp.effPoint += 1;
        }
        if (( ADCM->monitor.adcoriginal.cp.zeroPoint + ADCM->monitor.adcoriginal.cp.effPoint) >= ADC_CP_FB_AMP_LIST_CAL_LEN)
        {
            ADCM->monitor.adcoriginal.cp.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_CP_update.
*\fun     计算 CP
*\param   ADCM_info_t *ADCM
*\return  none
CP  5V
3.3K 2K分压 约二极管后端 2V 二极管前端2.7V , 此Zeroline以下均认为是CP = 0
5 4095
Vout = (ADCM->monitor.adcoriginal.cp.adc_value * (5.0f / 4095.0f)) * ((3.3f+2.0f)/2)
*/
static void AdcM_cal_CP_update(ADCM_info_t *ADCM)
{
    if(ADCM->monitor.adcoriginal.cp.startFlag == 0) /* =1 开始计算*/
    {
        if(ADCM->debug.cp.flag)
        {
            AdcM_ADC_Val.AdcM_CP_Val = ADCM->debug.cp.Value; // 串口输入 = 实际电压*100, 例如11.85V输入1185
        }
        else
        {
            for(int i =0; i < ADCM->monitor.adcoriginal.cp.effPoint;i++)
            {
                ADCM->monitor.adcoriginal.cp.sum += ADCM->monitor.adcoriginal.cp.adcList[i];
            }
            if(ADCM->monitor.adcoriginal.cp.effPoint > 0)
            {
                ADCM->monitor.adcoriginal.cp.adc_average = ADCM->monitor.adcoriginal.cp.sum / (ADCM->monitor.adcoriginal.cp.effPoint);
            }
            else
            {
                ADCM->monitor.adcoriginal.cp.adc_average =0;
            }
            // 实际电压 = ADC采样电压 / 分压比
            AdcM_ADC_Val.AdcM_CP_Val = (uint32_t)((((ADCM->monitor.adcoriginal.cp.adc_average * 5.0f / 4095) * ((3.3f+2.0f)/2.0f))+0.6f)*100); //0.6二极管补偿,cp放大100倍
        }
        ADCM->monitor.adcoriginal.cp.effPoint = 0;
        ADCM->monitor.adcoriginal.cp.zeroPoint = 0;
        ADCM->monitor.adcoriginal.cp.sum        = 0;
        memset(ADCM->monitor.adcoriginal.cp.adcList,0,sizeof(ADCM->monitor.adcoriginal.cp.adcList));
        ADCM->monitor.adcoriginal.cp.startFlag = 1;
    }
}

/**
*\name    AdcM_get_NTC1Temp_update.
*\fun     采样 NTC1Temp
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_NTC1Temp_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.ntc1.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.ntc1.startFlag == 1) /* =1 开始监测*/
    {
        if (ADCM->monitor.adcoriginal.ntc1.effPoint < ADC_TEMP_NTC1_LIST_CAL_LEN)
        {
            ADCM->monitor.adcoriginal.ntc1.adcList[ADCM->monitor.adcoriginal.ntc1.effPoint] = ADCM->monitor.adcoriginal.ntc1.adc = Samp_Val;
            ADCM->monitor.adcoriginal.ntc1.effPoint += 1;
        }else{
            ADCM->monitor.adcoriginal.ntc1.startFlag = 0;
        }
    }
}

/**
*\name    getTempInterp.
*\fun     插排温度查表法
*\param   targetVol
*\return  none
*/
// 进阶查表 (线性插值, 适配无精确匹配的情况)
static int32_t getTempInterpH(uint32_t targetVol) {
    /* 边界保护 */
    if (targetVol >= table1[0].voltage) return table1[0].temp;
    if (targetVol <= table1[TABLE1_SZ-1].voltage) return table1[TABLE1_SZ-1].temp;
    for(int i=0; i<(TABLE1_SZ-1); i++)
    {
        uint32_t voltage1 = table1[i].voltage;
        uint32_t voltage2 = table1[i+1].voltage;
        if (targetVol >= voltage2 && targetVol <= voltage1) {
            return table1[i].temp;
        }
    }
    return 999;//(异常标记)
}

// 进阶查表 (线性插值, 适配无精确匹配的情况)
static int32_t getTempInterpL(uint32_t targetVol) {
    /* 边界保护 */
    if (targetVol >= table2[0].voltage) return table2[0].temp;
    if (targetVol <= table2[TABLE2_SZ-1].voltage) return table2[TABLE2_SZ-1].temp;
    for(int i=0; i<(TABLE2_SZ-1); i++)
    {
        uint32_t voltage1 = table2[i].voltage;
        uint32_t voltage2 = table2[i+1].voltage;
        if (targetVol >= voltage2 && targetVol <= voltage1) {
            return table2[i].temp;
        }
    }
    return 999;//(异常标记)
}

static void AdcM_cal_NTC1Temp_update(ADCM_info_t *ADCM)
{
    if(ADCM->monitor.adcoriginal.ntc1.startFlag == 0) /* =1 开始计算*/
    {
        for(int i = 0; i < ADCM->monitor.adcoriginal.ntc1.effPoint; i++)
        {
            ADCM->monitor.adcoriginal.ntc1.sum += ADCM->monitor.adcoriginal.ntc1.adcList[i];
        }
        ADCM->monitor.adcoriginal.ntc1.adc_average = ADCM->monitor.adcoriginal.ntc1.sum / ADCM->monitor.adcoriginal.ntc1.effPoint;
        if(ADCM->debug.ntc1vol.flag)
        {
            AdcM_ADC_Val.AdcM_NTC1_Val = ADCM->debug.ntc1vol.Value;
        }
        else
        {
            AdcM_ADC_Val.AdcM_NTC1_Val = (uint32_t)((ADCM->monitor.adcoriginal.ntc1.adc_average*5.0f/4095)*1000);
        }

        if(ADCM->debug.ntc1temp.flag)
        {
            AdcM_ADC_Val.AdcM_NTC1Temp_Val = ADCM->debug.ntc1temp.Value;
        }
        else
        {
            if(AdcM_ADC_Val.AdcM_NTC1_Val >2000)//充电2KW 充电3.5KW 放电插排
            {
                AdcM_ADC_Val.AdcM_NTC1Temp_Val = getTempInterpH(AdcM_ADC_Val.AdcM_NTC1_Val);
            }
            else //充电3.5KW带转接头, 16A改为限流8A
            {
                AdcM_ADC_Val.AdcM_NTC1Temp_Val = getTempInterpL(AdcM_ADC_Val.AdcM_NTC1_Val);
            }
        }
        ADCM->monitor.adcoriginal.ntc1.sum = 0;
        ADCM->monitor.adcoriginal.ntc1.effPoint = 0;
        ADCM->monitor.adcoriginal.ntc1.startFlag = 1;
    }
}

/**
*\name    AdcM_get_Ground_update.
*\fun     采样 Ground
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_Ground_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.ground.adc = Samp_Val;
//    printf(" ADCM->monitor.adcoriginal.pgd.adc = %d\r\n",  ADCM->monitor.adcoriginal.pgd.adc );
    if(ADCM->monitor.adcoriginal.ground.startFlag == 1) /* =1 开始监测*/
    {
        if(ADCM->monitor.adcoriginal.ground.adc < 400)
        {
            ADCM->monitor.adcoriginal.ground.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.ground.effPoint++;
        }
        if(ADCM->monitor.adcoriginal.ground.zeroPoint + ADCM->monitor.adcoriginal.ground.effPoint > ADC_PGD_LEN)
        {
            ADCM->monitor.adcoriginal.ground.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_Ground_update.
*\fun     计算 Ground
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_Ground_update(ADCM_info_t *ADCM)
{
    if(ADCM->monitor.adcoriginal.ground.startFlag == 0) /* =0 开始计算*/
    {
        if (ADCM->debug.ground.flag)
        {
            AdcM_ADC_Val.AdcM_Ground_Sts = ADCM->debug.ground.pgdstatus;
        }
        else
        {
            if(ADCM->monitor.adcoriginal.ground.effPoint > ADC_PGD_Threadhold_LEN)
            {
                AdcM_ADC_Val.AdcM_Ground_Sts = AdcM_PGD_STATUS_OK;
            }
            else
            {
                AdcM_ADC_Val.AdcM_Ground_Sts = AdcM_PGD_STATUS_ERR;
            }
        
        ADCM->monitor.adcoriginal.ground.zeroPoint = 0;
        ADCM->monitor.adcoriginal.ground.effPoint = 0;
        ADCM->monitor.adcoriginal.ground.startFlag = 1;
    }
}
}
/**
*\name    AdcM_get_EvdiodeCHK_update.
*\fun     采样 EvdiodeCHK
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_EvdiodeCHK_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.evdio.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.evdio.startFlag == 1) /* =1 开始监测*/
    {
        // zeroLine = 600;  //(600/4095*13.9/3.9 *5 ≈ 2.6V以上)
        if (ADCM->monitor.adcoriginal.evdio.adc < 600) //(600*(5/4095))*((13.9f+3.9f)/3.9) = 2.61V 合并
        {
            ADCM->monitor.adcoriginal.evdio.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.evdio.adcList[ADCM->monitor.adcoriginal.evdio.effPoint] = ADCM->monitor.adcoriginal.evdio.adc;
            ADCM->monitor.adcoriginal.evdio.effPoint += 1;
        }
        if (( ADCM->monitor.adcoriginal.evdio.zeroPoint + ADCM->monitor.adcoriginal.evdio.effPoint) > ADC_EVDIODE_CHK_LEN)
        {
            ADCM->monitor.adcoriginal.evdio.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_EvdiodeCHK_update.
*\fun     计算 EvdiodeCHK
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_EvdiodeCHK_update(ADCM_info_t *ADCM)
{
    if(ADCM->monitor.adcoriginal.evdio.startFlag == 0) /* =1 开始计算*/
    {
        uint16_t min_val, max_val;
        if(ADCM->debug.evdio.flag)
        {
            AdcM_ADC_Val.AdcM_EvdiodeCHK_Sts = ADCM->debug.evdio.Value;
        }
        else
        {
            for (int i = 0; i < ADCM->monitor.adcoriginal.evdio.effPoint; i++) {
                if (i == 0) {
                    min_val = max_val = ADCM->monitor.adcoriginal.evdio.adcList[i];
                } else {
                    if (ADCM->monitor.adcoriginal.evdio.adcList[i] < min_val) min_val = ADCM->monitor.adcoriginal.evdio.adcList[i];
                    if (ADCM->monitor.adcoriginal.evdio.adcList[i] > max_val) max_val = ADCM->monitor.adcoriginal.evdio.adcList[i];
                }
                ADCM->monitor.adcoriginal.evdio.sum += ADCM->monitor.adcoriginal.evdio.adcList[i];
            }
            ADCM->monitor.adcoriginal.evdio.sum = ADCM->monitor.adcoriginal.evdio.sum - min_val - max_val;
            if(ADCM->monitor.adcoriginal.evdio.effPoint > 2)
            {
                ADCM->monitor.adcoriginal.evdio.adc_average = ADCM->monitor.adcoriginal.evdio.sum / (ADCM->monitor.adcoriginal.evdio.effPoint - 2);
            }
            else
            {
                ADCM->monitor.adcoriginal.evdio.adc_average =0;
            }
            // 实际电压 = ADC采样电压 / 分压比
            ADCM->monitor.adcoriginal.evdio.val = (uint32_t)(((ADCM->monitor.adcoriginal.evdio.adc_average * (5.0f / 4095.0f)) * ((10.0f+3.9f)/3.9f))*100);
        }
        if((ADCM->monitor.adcoriginal.evdio.val < 900) || (ADCM->monitor.adcoriginal.evdio.val <1320))
        {
            AdcM_ADC_Val.AdcM_EvdiodeCHK_Sts = AdcM_EVDIODE_STATUS_ERR;
        }
        else
        {
            AdcM_ADC_Val.AdcM_EvdiodeCHK_Sts = AdcM_EVDIODE_STATUS_OK;
        }
        ADCM->monitor.adcoriginal.evdio.effPoint = 0;
        ADCM->monitor.adcoriginal.evdio.zeroPoint = 0;
        ADCM->monitor.adcoriginal.evdio.sum       = 0;
        memset(ADCM->monitor.adcoriginal.evdio.adcList,0,sizeof(ADCM->monitor.adcoriginal.evdio.adcList));
        ADCM->monitor.adcoriginal.evdio.startFlag = 1;
    }
}

/**
*\name    AdcM_get_RelayCHK_update.
*\fun     采样 RelayCHK
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_RelayCHK_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.rlychk.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.rlychk.startFlag == 1) /* =1 开始监测*/
    {
        if( ADCM->monitor.adcoriginal.rlychk.adc < 680)
        {
            ADCM->monitor.adcoriginal.rlychk.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.rlychk.effPoint++;
        }
        if(ADCM->monitor.adcoriginal.rlychk.zeroPoint + ADCM->monitor.adcoriginal.rlychk.effPoint > ADC_RLY_CHK_MCU_LEN)
        {
            ADCM->monitor.adcoriginal.rlychk.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_RelayCHK_update.
*\fun     计算 RelayCHK
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_RelayCHK_update(ADCM_info_t *ADCM)
{
    if(ADCM->monitor.adcoriginal.rlychk.startFlag == 0) /* =0 开始计算*/
    {
        if (ADCM->debug.rlychk.flag)
        {
            AdcM_ADC_Val.AdcM_RelayCHK_Sts = ADCM->debug.rlychk.rlychkstatus;
        }
        else
        {
            if(ADCM->monitor.adcoriginal.rlychk.effPoint > ADC_RLY_CHK_Threadhold_LEN)
            {
                AdcM_ADC_Val.AdcM_RelayCHK_Sts = AdcM_RLY_CHK_STATUS_CONNECT;
            }

        else
        {
            AdcM_ADC_Val.AdcM_RelayCHK_Sts = AdcM_RLY_CHK_STATUS_BREAK;
        }
        ADCM->monitor.adcoriginal.rlychk.zeroPoint = 0;
        ADCM->monitor.adcoriginal.rlychk.effPoint = 0;
        ADCM->monitor.adcoriginal.rlychk.startFlag = 1;
    }
}
}
/**
*\name    AdcM_get_MCUTemp_update.
*\fun     采样 MCUTemp
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_MCUTemp_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.mcutemp.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.mcutemp.startFlag == 1) /* =1 开始监测*/
    {
        if (ADCM->monitor.adcoriginal.mcutemp.effPoint < ADC_MCU_Temp_LEN)
        {
            ADCM->monitor.adcoriginal.mcutemp.adcList[ADCM->monitor.adcoriginal.mcutemp.effPoint] = ADCM->monitor.adcoriginal.mcutemp.adc;
            ADCM->monitor.adcoriginal.mcutemp.effPoint += 1;
        }else{
            ADCM->monitor.adcoriginal.mcutemp.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_MCUTemp_update.
*\fun     计算 RelayCHK
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_MCUTemp_update(ADCM_info_t *ADCM)
{
    uint32_t TSValue;
    float tempValue,VTS,T_value;;
    if(ADCM->monitor.adcoriginal.mcutemp.startFlag == 0) /* =0 开始计算*/
    {
        if(ADCM->debug.mcutemp.flag)
        {
            AdcM_ADC_Val.AdcM_MCUTemp_Val = ADCM->debug.mcutemp.Value;
        }
        else
        {
            ADCM->monitor.adcoriginal.mcutemp.adc_average = ADCF_CalculateIntAverage_Conversion(ADCM->monitor.adcoriginal.mcutemp.adcList,ADC_MCU_Temp_LEN);//取平均值
            tempValue = ADCM->monitor.adcoriginal.mcutemp.adc_average*(VREF_VALUE/4095);
            TSValue= (*(uint32_t*)(VTS_CODE_ADDR)) & 0xFFFF;
            #ifdef VDDA_5V
            VTS = TSValue*(5.0/4095);
            #else
            VTS = TSValue*(3.3/4095);
            #endif
            T_value  = (uint16_t) (*(uint32_t *)TS_VALUE_ADDR);
            if(T_value == 0xFFFF)
            {
                T_value = 25000;
            }
            else
            {
                /* no process */
            }
            /* Get the temperature inside the chip */
            AdcM_ADC_Val.AdcM_MCUTemp_Val = (((VTS-tempValue)/AVG_SLOPE + T_value/1000.0f)*100);
        }
        ADCM->monitor.adcoriginal.mcutemp.effPoint = 0;
        ADCM->monitor.adcoriginal.mcutemp.adc_average = 0;
        memset(ADCM->monitor.adcoriginal.mcutemp.adcList,0,sizeof(ADCM->monitor.adcoriginal.mcutemp.adcList));
        ADCM->monitor.adcoriginal.mcutemp.startFlag = 1;
    }
}

/**
*\name    AdcM_get_ShortCircuit_update.
*\fun     采样 ShortCircuit
*\param   ADCM_info_t *ADCM,uint16_t Samp_Val
*\return  none
*/
void AdcM_get_ShortCircuit_update(ADCM_info_t *ADCM, uint16_t Samp_Val)
{
    ADCM->monitor.adcoriginal.shortcircuit.adc = Samp_Val;
    if(ADCM->monitor.adcoriginal.shortcircuit.startFlag == 1) /* =1 开始监测*/
    {
        if(ADCM->monitor.adcoriginal.shortcircuit.adc > 2865) //小于3.5V
        {
            ADCM->monitor.adcoriginal.shortcircuit.zeroPoint++;
        }
        else
        {
            ADCM->monitor.adcoriginal.shortcircuit.effPoint++;
        }
        if(ADCM->monitor.adcoriginal.shortcircuit.zeroPoint + ADCM->monitor.adcoriginal.shortcircuit.effPoint > ADC_SHORTCIRCUIT_LEN)
        {
            ADCM->monitor.adcoriginal.shortcircuit.startFlag = 0;
        }
    }
}

/**
*\name    AdcM_cal_ShortCircuit_update.
*\fun     计算 ShortCircuit
*\param   ADCM_info_t *ADCM
*\return  none
*/
static void AdcM_cal_ShortCircuit_update(ADCM_info_t *ADCM)
{
    if(ADCM->monitor.adcoriginal.shortcircuit.startFlag == 0) /* =0 开始计算*/
    {
        if (ADCM->debug.shortcircuit.flag)
        {
            AdcM_ADC_Val.AdcM_shortcircuit_Sts = ADCM->debug.shortcircuit.shortcircuitstatus;
        }
        else
        {
            if(ADCM->monitor.adcoriginal.shortcircuit.effPoint > ADC_SHORTCIRCUIT_Threadhold_LEN)
            {
                AdcM_ADC_Val.AdcM_shortcircuit_Sts = AdcM_SHORTCIRCUIT_STATUS_ERR;
            }
            else
            {
                AdcM_ADC_Val.AdcM_shortcircuit_Sts = AdcM_SHORTCIRCUIT_STATUS_OK;
            }
            ADCM->monitor.adcoriginal.shortcircuit.zeroPoint = 0;
            ADCM->monitor.adcoriginal.shortcircuit.effPoint = 0;
            ADCM->monitor.adcoriginal.shortcircuit.startFlag = 1;
        }
    }
}

/**
*\name    AdcM_CallbackSampleValue.
*\fun     数据计算处理
*\param   none
*\return  none
*/
void AdcM_CallbackSampleValue(void)
{
    AdcM_cal_Voltage_update(&ADCM_t);
    AdcM_cal_Current_update(&ADCM_t);
    AdcM_cal_CP_update(&ADCM_t);
    AdcM_cal_NTC1Temp_update(&ADCM_t);
    AdcM_cal_Ground_update(&ADCM_t);
    AdcM_cal_EvdiodeCHK_update(&ADCM_t);
    AdcM_cal_RelayCHK_update(&ADCM_t);
    AdcM_cal_MCUTemp_update(&ADCM_t);
    AdcM_cal_ShortCircuit_update(&ADCM_t);
}

/**
*\name    AdcM_GetValidValue.
*\fun     Gets Valid Value
*\param   AdCfg_ChanEnumType ucChannel
*\return  none
*/

int32_t AdcM_GetValidValue(AdcCfg_ChanEnumType ucChannal)
{
    int32_t ucChannel_Val;
    switch(ucChannal)
    {
        case ADCMCFG_Voltage_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_Voltage_Val;
            break;
        case ADCMCFG_Currnet_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_Current_Val;
            break;
        case ADCMCFG_CP_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_CP_Val;
            break;
        case ADCMCFG_NTC1_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_NTC1_Val;
            break;
        case ADCMCFG_NTC1Temp_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_NTC1Temp_Val;
            break;
        case ADCMCFG_Ground_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_Ground_Sts;
            break;
        case ADCMCFG_EvdiodeCHK_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_EvdiodeCHK_Sts;
            break;
        case ADCMCFG_RelayCHK_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_RelayCHK_Sts;
            break;
        case ADCMCFG_MCUTemp_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_MCUTemp_Val;
            break;
        case ADCMCFG_Shortcircuit_CH:
            ucChannel_Val = AdcM_ADC_Val.AdcM_shortcircuit_Sts;
            break;
        case ADCMCFG_RMSVOLX1000:
            ucChannel_Val = AdcM_ADC_Val.AdcM_VOL_RMSx1000;
            break;
        case ADCMCFG_RMSCURX1000:
            ucChannel_Val = AdcM_ADC_Val.AdcM_CUR_RMSx1000;
            break;
        default:
            break;
    }
    return ucChannel_Val;
}
