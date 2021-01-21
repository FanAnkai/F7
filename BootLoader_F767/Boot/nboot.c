/**
 * @file nboot.c
 * @author Segway Inside 
 * @brief 和 IAP 固件升级有关的函数
 * @version 0.2
 * @date 2020-03-06
 * 
 * @copyright Copyright (c) 2020 纳恩博（北京）科技有限公司
 * All rights reserved.
 * 
 */
#include "nboot.h"
#include "flash.h"

#if defined(RTT_ENABLE)
#include "SEGGER_RTT.h"
#endif

#if defined(NBOOT_DEBUG) && defined(RTT_ENABLE)
#define Log(...) RTT_printf(__VA_ARGS__)
#else
#define Log(...)
#endif

#if defined(RTT_ENABLE)
int RTT_printf(const char * sFormat, ...)
{
    int r;
    va_list param_list;
    
    va_start(param_list, sFormat);
    r = SEGGER_RTT_vprintf(0, sFormat, &param_list);
    va_end(param_list);    
    
    return r;
}
#endif

//boot
///////////////////////////////////////////////////////////////////////////
//static void system_reboot(void)
//{
//    NVIC_SystemReset();
//}

static void disable_irq(void)
{
    __set_PRIMASK(1);
}

static void system_jump_to(uint32_t ApplicationAddress)
{
    uint32_t JumpAddress;
    
    //hardware_deinit();
	
	
    disable_irq();
    JumpAddress = *(uint32_t *)(ApplicationAddress + 4);
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(uint32_t *)ApplicationAddress);

    (*((void (*)(void))JumpAddress))();
}

static int iap_jump_to(uint32_t ApplicationAddress)
{
    Log("jump to app...\r\n");
    if ((*(uint32_t *)ApplicationAddress) < NBOOT_SRAM_END)
    {
        system_jump_to(ApplicationAddress);
        return 1;
    }

    return 0;
}



#define IAP_BUF_SIZE    256
void boot_app(const Temp_Flash_Context *flash_ctx)
{
    int ret = 0;
    uint8_t iap_buf[IAP_BUF_SIZE];
    uint8_t down_state = 0;
    uint32_t down_size = 0;
    uint32_t img_read_addr = 0;
    uint32_t img_write_addr = IMAGE_SLOT_A_START;
    
    ret = nboot_get_image_down_state(&down_state, &down_size);
    if (ret < 0)
    {
        goto err;
    }
    Log("down_state %d, down_size %d\r\n", down_state, down_size);
    
    if (down_state == IMAGE_STATE_DOWNLOAD_OK && down_size > 0)
    {
        ret = flash_erase(IMAGE_SLOT_A_START, down_size);
        if (ret < 0)
        {
            goto err;
        }
        img_read_addr = flash_ctx->flash_start_addr;
        for (int i = 0; i < down_size; i += IAP_BUF_SIZE)
        {
            ret = flash_ctx->backend->read(img_read_addr, iap_buf, IAP_BUF_SIZE);
            if (ret < 0)
            {
                goto err;
            }
            ret = flash_write(img_write_addr, iap_buf, IAP_BUF_SIZE);
            if (ret < 0)
            {
                goto err;
            }
            img_read_addr += IAP_BUF_SIZE;
            img_write_addr += IAP_BUF_SIZE;
        }
        ret = nboot_set_image_down_state(IMAGE_STATE_INVALID, 0);
        if (ret < 0)
        {
            goto err;
        }
    }

    iap_jump_to(IMAGE_SLOT_A_START);

err:
    Log("boot app failed!\r\n");
    while (1)
    {
    }
}




//slot
///////////////////////////////////////////////////////////////////////////
static uint8_t buffer[IMAGE_META_DATA_MAX_SIZE] = {0};

const Slot_Range down_img_slot =
{
    .start = IMAGE_SLOT_B_START,
    .end = IMAGE_SLOT_B_END
};

static const IAP_Images_Down_State default_state = 
{
    .magic_bytes = IMAGE_DOWN_MAGIC,
    .down_state = IMAGE_STATE_INVALID,
    .down_size = 0
};

static int load_image_down_state(IAP_Images_Down_State *state)
{
    int ret = 0;

    if (state == NULL)
    {
        return -1;
    }

    ret = flash_read(IMAGE_META_DATA_ADDR, (uint8_t *)state, sizeof(IAP_Images_Down_State));
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}


static int save_image_down_state(IAP_Images_Down_State *state)
{
    int ret = 0;

    if (state == NULL)
    {
        return -1;
    }

    ret = flash_read(IMAGE_META_DATA_ADDR, buffer, IMAGE_META_DATA_MAX_SIZE);
    if (ret < 0)
    {
        goto err;
    }
    memcpy(buffer, state, sizeof(IAP_Images_Down_State));
    ret = flash_erase(IMAGE_META_DATA_ADDR, IMAGE_META_DATA_MAX_SIZE);
    if (ret < 0)
    {
        goto err;
    }
    ret = flash_write(IMAGE_META_DATA_ADDR, buffer, IMAGE_META_DATA_MAX_SIZE);
    if (ret < 0)
    {
        goto err;
    }
    return 0;

err:
    return -1;
}

const Slot_Range *nboot_get_down_slot(void)
{
    return &down_img_slot;
}

int nboot_get_image_down_state(uint8_t *down_state, uint32_t *down_size)
{
    int ret = 0;
    IAP_Images_Down_State state = {0};

    ret = load_image_down_state(&state);
    if (ret < 0)
    {
        goto err;
    }

    if (state.magic_bytes == IMAGE_DOWN_MAGIC)
    {
        *down_state = state.down_state;
        *down_size = state.down_size;
    }
    else
    {
        *down_state = default_state.down_state;
        *down_size = default_state.down_size;
    }

    return 0;
err:
    return -1;
}

int nboot_set_image_down_state(uint8_t down_state, uint32_t down_size)
{
    int ret = 0;
    IAP_Images_Down_State state = {0};
    uint8_t need_update = 0;

    ret = load_image_down_state(&state);
    if (ret < 0)
    {
        goto err;
    }
    if (state.magic_bytes == IMAGE_DOWN_MAGIC)
    {
        if (state.down_state != down_state 
            || state.down_size != down_size)
        {
            state.down_state = down_state;
            state.down_size = down_size;
            need_update = 1;
        }
    }
    else
    {
        state = default_state;
        need_update = 1;
    }

    if (need_update != 0)
    {
        ret = save_image_down_state(&state);
        if (ret < 0)
        {
            goto err;
        }
    }

    return 0;
err:
    return -1;
}

