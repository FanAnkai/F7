#include "module_store.h"
#include "ql_include.h"

iot_s32 _config_save(iot_u8 *pCfg, iot_u32 Len)
{
    if(NULL == pCfg || Len > CONFIG_TOTAL_LEN)
    {
        return -1;
    }

    iot_local_save(Len, (void*)pCfg);

    return 0;
}

iot_s32 _config_load(iot_u8 *pCfg, iot_u32 Len)
{
    iot_s32 ret = 0;
    if(NULL == pCfg || Len > CONFIG_TOTAL_LEN)
    {
        return -1;
    }

    ret = iot_local_load(Len, (void*)pCfg);

    return ret;
}

iot_s32 user_config_erase()
{
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];

    memset(CfgBuf, 0xFF, CONFIG_TOTAL_LEN);
    _config_save(CfgBuf, CONFIG_TOTAL_LEN);

    return 0;
}

void product_config_save(PRODUCT_CFG* pProductCfg)
{
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];
    if(NULL == pProductCfg)
    {
        return;
    }

    _config_load(CfgBuf, CONFIG_TOTAL_LEN);
    memcpy(&CfgBuf[PRODUCT_CFG_OFFSET], pProductCfg, PRODUCT_CFG_LEN);
    _config_save(CfgBuf, CONFIG_TOTAL_LEN);
}

iot_s32 product_config_load(PRODUCT_CFG* pProductCfg)
{
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];
    iot_s32 ret = 0;
    if(NULL == pProductCfg)
    {    
        return;
    }
    
    ret = _config_load(CfgBuf, CONFIG_TOTAL_LEN);
    memcpy(pProductCfg, &CfgBuf[PRODUCT_CFG_OFFSET], PRODUCT_CFG_LEN);

    return ret;
}

void product_config_erase()
{
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];

    _config_load(CfgBuf, CONFIG_TOTAL_LEN);
    memset(&CfgBuf[PRODUCT_CFG_OFFSET], 0xFF, PRODUCT_CFG_LEN);
    _config_save(CfgBuf, CONFIG_TOTAL_LEN);
}

void startup_config_save(STARTUP_CFG* pStartupCfg)
{
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];

    if(NULL == pStartupCfg)
    {
        return;
    }

    _config_load(CfgBuf, CONFIG_TOTAL_LEN);
    memcpy(&CfgBuf[STARTUP_CFG_OFFSET], pStartupCfg, STARTUP_CFG_LEN);
    _config_save(CfgBuf, CONFIG_TOTAL_LEN);
}

iot_s32 startup_config_load(STARTUP_CFG* pStartupCfg)
{
    iot_s32 ret = 0;
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];

    if(NULL == pStartupCfg)
    {
        return;
    }
    
    ret = _config_load(CfgBuf, CONFIG_TOTAL_LEN);
    memcpy(pStartupCfg, &CfgBuf[STARTUP_CFG_OFFSET], STARTUP_CFG_LEN);

    return ret;
}

void startup_config_erase()
{
    iot_u8 CfgBuf[CONFIG_TOTAL_LEN];

    _config_load(CfgBuf, CONFIG_TOTAL_LEN);
    memset(&CfgBuf[STARTUP_CFG_OFFSET], 0xFF, STARTUP_CFG_LEN);
    _config_save(CfgBuf, CONFIG_TOTAL_LEN);
}

iot_s32 product_init_config(PRODUCT_CFG * pstProductCfg)
{
    iot_s32 ret = 0;
    PRODUCT_CFG LocalProductCfg;
    memset(&LocalProductCfg, 0x00, sizeof(LocalProductCfg));
    
    ret = product_config_load(&LocalProductCfg);

    if (ret == -1 || LocalProductCfg.product_id == -1 || LocalProductCfg.product_key[0] == 255 || LocalProductCfg.mcu_ver[0] == 255 )
    {
        product_config_save(pstProductCfg);
        wifi_config_erase();
        startup_config_erase();
        ql_log_info("CFG save, [init]\r\n");
    }
    else if(0 != strcmp(pstProductCfg->mcu_ver, LocalProductCfg.mcu_ver)) // different version
    {
        if( strcmp(pstProductCfg->mcu_ver, LocalProductCfg.mcu_ver) > 0) // new version
        {
            product_config_save(pstProductCfg);
            ql_log_info("CFG save, [new]\r\n");
        }
        else // old version
        {
            ql_log_info("CFG is old, [%c%c%c%c%c]\r\n", LocalProductCfg.mcu_ver[0], 
                LocalProductCfg.mcu_ver[1], LocalProductCfg.mcu_ver[2],
                LocalProductCfg.mcu_ver[3], LocalProductCfg.mcu_ver[4]);
            return -1;
        }
    }
    else // same version
    {
    }

    ql_log_info("CFG init ok\r\n");
    return 0;
}
