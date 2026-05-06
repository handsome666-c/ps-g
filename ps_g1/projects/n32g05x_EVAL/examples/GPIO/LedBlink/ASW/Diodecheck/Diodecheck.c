#include "Diodecheck.h"

// 初始化自检管理器
void APP_DiodeCheck_Init(void) {
    diodeCheckManager.state = DIODECHECK_IDLE;
    diodeCheckManager.timer = 0;
    diodeCheckManager.errorCode = DIODECHECK_ERROR_NONE;
    diodeCheckManager.currentTestIndex = 0;
    diodeCheckManager.subState = DIODECHECK_SUBSTATE_IDLE;

    for (int i = 0; i < DIODECHECK_INDEX_MAX; i++) {
        diodeCheckManager.testResults[i] = false;
        diodeCheckManager.faultStatus[i] = false;
    }
}

// 开始自检
void APP_DiodeCheck_Start(void) {
    if (diodeCheckManager.state == DIODECHECK_IDLE) {
        diodeCheckManager.state = DIODECHECK_RUNNING;
        diodeCheckManager.timer = 0;
        diodeCheckManager.currentTestIndex = DIODECHECK_INDEX;
        diodeCheckManager.errorCode = DIODECHECK_ERROR_NONE;
        diodeCheckManager.subState = DIODECHECK_SUBSTATE_START;
    }
}

// 切换到下一个测试项目
static void APP_DiodeCheck_Next_Test(void) {
    diodeCheckManager.currentTestIndex++;
    diodeCheckManager.subState = DIODECHECK_SUBSTATE_START;
    diodeCheckManager.timer = 0;

    if (diodeCheckManager.currentTestIndex >= DIODECHECK_INDEX_MAX) {
        diodeCheckManager.currentTestIndex = DIODECHECK_INDEX_MAX; // 进入完成状态
    }
}

void APP_Diode_Check(void) {
    // 1. 读取硬件IO或ADC值
    bool currentFaultStatus = false;
    if (RTE_Call_AdcM_Vlotage(ADCMCFG_EvdiodeCHK_CH) == 1)
    {
        currentFaultStatus = true;
    }

    // 2. 实现消抖逻辑
    static uint8_t debounceCounter = 0;

    if (currentFaultStatus) {
        if (debounceCounter < DIODECHECK_DEBOUNCE_THRESHOLD) {
            debounceCounter++;
        }
    } else {
        debounceCounter = 0; // 一旦发现没问题，计数器清零
    }

    // 3. 更新最终状态
    diodeCheckManager.faultStatus[DIODECHECK_INDEX] = (debounceCounter >= DIODECHECK_DEBOUNCE_THRESHOLD);
}

bool APP_DiodeCheck_GetFaultStatus(void) {
    return diodeCheckManager.faultStatus[DIODECHECK_INDEX];;
}

// 完成自检
static void APP_DiodeCheck_Complete(void) {
    bool allPassed = true;
    event_fault_charging_data_t st_event_fault_data;
    for (int i = 0; i < DIODECHECK_INDEX_MAX; i++) {
        if (diodeCheckManager.testResults[i]) {
            allPassed = false;
            // errorCode = i;
            break;
        }
    }

    if (allPassed) {
        rt_kprintf("DBG_INFO, DIAG, DIODECHECK_PASSED\r\n");
        diodeCheckManager.state = DIODECHECK_PASSED;
        APP_CHG_EventPubTopic(E_EVENT_CHG_DIODECHECK_COMPLETE); // 自检ok后进入二极管检测ok状态
    } else {
        rt_kprintf("DBG_INFO, DIAG, DIODECHECK_FAILED\r\n");
        diodeCheckManager.state = DIODECHECK_FAILED;
        st_event_fault_data.type = E_CHARGING_FAULT_DIODE_CHECK;
        APP_CHG_EventPub(E_EVENT_CHG_DIODECHECK_FAILURE, st_event_fault_data.type);
    }
    diodeCheckManager.state = DIODECHECK_IDLE;
}

// 自检状态机更新（在5ms定时器中断中调用）
void APP_DiodeCheck_Update(void) {
    if (diodeCheckManager.state != DIODECHECK_RUNNING) {
        return;
    }

    diodeCheckManager.timer += 5;

    switch (diodeCheckManager.currentTestIndex) {
        case DIODECHECK_INDEX:
            switch(diodeCheckManager.subState){
                case DIODECHECK_SUBSTATE_START:
                    diodeCheckManager.subState = DIODECHECK_SUBSTATE_CHECKING;
                    diodeCheckManager.timer = 0;
                    diodeCheckManager.faultStatus[DIODECHECK_INDEX] = false;
                    break;
                case DIODECHECK_SUBSTATE_CHECKING:
                    APP_Diode_Check();
                    if (diodeCheckManager.timer >= DIODECHECK_DURATION) { // 200ms检测时间
                        // 最终检查经过消抖后的状态
                        diodeCheckManager.faultStatus[DIODECHECK_INDEX] = APP_DiodeCheck_GetFaultStatus();
                        if(diodeCheckManager.faultStatus[DIODECHECK_INDEX] == true) {
                            diodeCheckManager.testResults[DIODECHECK_INDEX] = true;
                            diodeCheckManager.errorCode = DIODECHECK_ERROR;
                            diodeCheckManager.currentTestIndex = DIODECHECK_INDEX_MAX;
                        }
                        APP_DiodeCheck_Next_Test();
                    }
                    break;
                default:
                    diodeCheckManager.subState = DIODECHECK_SUBSTATE_START;
                    break;
            }
            break;
        case DIODECHECK_INDEX_MAX: // 自检完成
            APP_DiodeCheck_Complete();
            break;
    }
}
