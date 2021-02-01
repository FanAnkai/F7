#ifndef __MODULE_STORE_H__
#define __MODULE_STORE_H__

#include "iot_interface.h"
#include "protocol.h"

#define PRODUCT_CFG_LEN  (sizeof(PRODUCT_CFG))
#define STARTUP_CFG_LEN  (sizeof(STARTUP_CFG))

#define CONFIG_TOTAL_LEN  (PRODUCT_CFG_LEN + STARTUP_CFG_LEN + MODULE_CFG_LEN)

#define PRODUCT_CFG_OFFSET  (0)
#define STARTUP_CFG_OFFSET  (PRODUCT_CFG_OFFSET + PRODUCT_CFG_LEN)
#define MODULE_CFG_OFFSET     (PRODUCT_CFG_OFFSET + PRODUCT_CFG_LEN + STARTUP_CFG_LEN)

typedef struct
{
    iot_u32 product_id;
    iot_u8  product_key[PRODUCT_KEY_LEN];
    iot_u8  mcu_ver[MCU_VER_LEN];
}PRODUCT_CFG;

typedef struct
{
    iot_u8 mode;
    iot_u8 timeout;
}STARTUP_CFG;

iot_s32 _config_save(iot_u8 *pCfg, iot_u32 Len);
iot_s32 _config_load(iot_u8 *pCfg, iot_u32 Len);
iot_s32 user_config_erase();
void product_config_save(PRODUCT_CFG* pProductCfg);
iot_s32 product_config_load(PRODUCT_CFG* pProductCfg);
void product_config_erase();
void startup_config_save(STARTUP_CFG* pStartupCfg);
iot_s32 startup_config_load(STARTUP_CFG* pStartupCfg);
void startup_config_erase();

iot_s32 product_init_config(PRODUCT_CFG * pstProductCfg);

#endif