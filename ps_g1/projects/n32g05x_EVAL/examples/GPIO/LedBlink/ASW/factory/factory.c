#include "factory.h"

factory_Manager_t factory_Manager;
// 和工厂模式相关的操作

uint32_t factory_code[2] = {0x11223344, 0x44332211};

void factory_entry_mode(factory_Manager_t *factory)
{
    FLASH_Unlock();
    FlsIf_ErasePage(FACTORY_INFO_ADDR);
    FlsIf_ProgramMultiple(FACTORY_INFO_ADDR, factory_code, 8);
    FLASH_Lock();
}

void factory_exit_mode(factory_Manager_t *factory)
{
    FLASH_Unlock();
    FlsIf_ErasePage(FACTORY_INFO_ADDR);
    FLASH_Lock();
}

bool get_factory_mode_status(factory_Manager_t *factory)
{
    FlsIf_ReadMultiple(FACTORY_INFO_ADDR, factory->readBuffer, 8);
    if((factory->readBuffer[0] == 0x11223344) && (factory->readBuffer[1] == 0x44332211))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void factory(int argc, char**argv)
{
    if(memcmp(argv[1],"help",strlen("help")) == 0)
    {
        rt_kprintf("interface:factory enter");
        rt_kprintf("interface:factory exit");
        rt_kprintf("interface:factory get");
    }
    if(memcmp(argv[1],"enter",strlen("enter")) == 0)
    {
        rt_kprintf("factory mode enter!\r\n");
        factory_entry_mode(&factory_Manager);
    }
    else if(memcmp(argv[1],"exit",strlen("exit")) == 0)
    {
        rt_kprintf("factory mode exit!\r\n");
        factory_exit_mode(&factory_Manager);
    }
    else if(memcmp(argv[1],"get",strlen("get")) == 0)
    {
        if(get_factory_mode_status(&factory_Manager))
        {
            rt_kprintf("With in the factory mode\r\n");
        }
        else
        {
            rt_kprintf("Not with in the factory mode\r\n");
        }
    }
    else
    {
        rt_kprintf("Invalid command! Usage: factory <enter/exit/get> ... ...\r\n");
    }
}
MSH_CMD_EXPORT(factory , "factory mode");

