#include "n32g05x.h"
#include "AdcF.h"

// 排序函数：从大到小排序
void sortDesc(uint16_t arr[], int n) {
    int i, j, temp;
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            if (arr[i] < arr[j]) {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

/*计算平均值*/
uint16_t ADCF_CalculateIntAverage_Conversion(uint16_t* data, uint8_t length)
{
    uint32_t sum = 0;  // 使用更大的数据类型防止溢出
    uint16_t i;
    for (i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum / length;  // 整数除法，结果向下取整
}

static __inline uint32_t FastSqrt( uint32_t input )
{
    uint32_t inputTemp = input;
    uint32_t rem = 0;
    uint32_t root = 0;
    uint32_t divisor = 0;
    uint8_t i;
    for( i = 0; i < 16; i++ )
    {
        root <<= 1;
        rem = ( (rem << 2) + (inputTemp >> 30) );
        inputTemp <<= 2;
        divisor = ( root << 1 ) | 0x01;
        if( divisor <= rem )
        {
            rem -= divisor;
            root++;
        }
    }
    return root;
}

uint32_t hard_root(uint32_t ori)
{
    uint32_t root;
    uint32_t result_root;
    root = FastSqrt(ori);
    result_root = root;
    return result_root;
}

uint8_t threadHold_check_u32(uint32_t q1, uint32_t q2, uint32_t threadhold)
{
    uint8_t tResult = 0;
    if (q1 > q2)
    {
        if ((q1 - q2) > threadhold)
        {
            tResult = 1;
        }
        else
        {
            tResult = 0;
        }
    }
    else
    {
        if ((q2 - q1) > threadhold)
        {
            tResult = 1;
        }
        else
        {
            tResult = 0;
        }
    }
    return tResult;
}
