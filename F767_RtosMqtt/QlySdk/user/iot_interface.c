/***************************************************************************************************
 * Filename: iot_interface.c
 * Description: user interface for iot mcu-sdk
 *
 * Modify:
 * 2017.12.7  hept Create.
 * 
 ***************************************************************************************************/
#include "iot_interface.h"
#include "protocol_if.h"
#include "module_config.h"
#include "spec_opcode.h"

int cloud_is_connected = 0;

//  MCU特定部分开始  ////////////////////////////////////////////////////////////

//MCU 请求命令的回复，判断请求是否成功
void iot_call_cb(iot_u8 opcode_value, iot_u8 result)
{
    printf("opcode_value:%02x, result:%d\r\n", opcode_value, result);
}

void iot_init_cb(iot_u8* ModuleMac, iot_u8* ModuleVersion)
{
    printf("%s, ", __FUNCTION__);
    printf("mac:%02X%02X%02X%02X%02X%02X, ", 
        ModuleMac[0], ModuleMac[1], ModuleMac[2], ModuleMac[3], ModuleMac[4], ModuleMac[5]);
    printf("ver:%c%c%c%c%c\r\n", 
        ModuleVersion[0], ModuleVersion[1], ModuleVersion[2], ModuleVersion[3], ModuleVersion[4]);

    cloud_is_connected = 0;
    iot_config_info(INFO_TYPE_SN, "123456");
    iot_ota_option_set(120, 1024, OTA_TYPE_BASIC);
}

void iot_module_status_cb(iot_u8 module_type, MODULE_STATUS status, iot_u32 timestamp)
{
    printf("module_type:%d, status:%d, timestamp:%d\r\n", module_type, status, timestamp);
    if(CLOUD_CONN_OK == status) {
        cloud_is_connected = 1;
    }
    else {
        cloud_is_connected = 0;
    }
}

void iot_module_set_mode_cb(iot_u8 mode, iot_u8 timeout, iot_u8 result)
{
    IOT_INFO("%s,%d, mode:%d, timeout:%d, result:%d\r\n", __func__, __LINE__, mode, timeout, result);
}

void iot_module_set_factory_cb(iot_u8 ret, iot_u8 rssi)
{
    IOT_INFO("factory ret:%d, rssi:%d\r\n", ret, rssi);
}

void iot_module_info_cb(iot_u8 info_type, iot_u8 info_len, iot_u8 * info)
{
    mdoule_info_get_ip get_ip;
    IOT_INFO("info_type:%02x, info_len:%d\r\n", info_type, info_len);

    switch (info_type)
    {
    case MODULE_INFO_TYPE_ROUTER:
        IOT_INFO("ssid:%s\r\n", info);
        break;
    case MODULE_INFO_TYPE_GETIP:
        if(info_len == sizeof(get_ip)) {
            memcpy(&get_ip, info, info_len);
            IOT_INFO("ip:%d.%d.%d.%d\r\n", get_ip.ip_addr[3], get_ip.ip_addr[2], get_ip.ip_addr[1], get_ip.ip_addr[0]);
            IOT_INFO("mask:%d.%d.%d.%d\r\n", get_ip.ip_mask[3], get_ip.ip_mask[2], get_ip.ip_mask[1], get_ip.ip_mask[0]);
            IOT_INFO("gw:%d.%d.%d.%d\r\n", get_ip.gateway[3], get_ip.gateway[2], get_ip.gateway[1], get_ip.gateway[0]);
        }
        break;
    default:
        break;
    }
}

//  MCU特定部分结束  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  第一部分 [系统函数]  /////////////////////////////////////////////////////

/*
 * 打印日志函数，请自行实现，可根据需要重定向到串口、屏幕、文件等位置
 */
void iot_print(const char *str)
{
    printf("%s", str);
}

/*
 * 当设备连接状态发生改变时，sdk自动调用此函数
 */
void iot_status_cb( DEV_STATUS_T dev_status, iot_u32 timestamp )
{
    iot_u32 seq = 0;
    printf("device state:%d, ts:%d\r\n", dev_status, timestamp);

    if( dev_status == DEV_STA_CONNECTED_CLOUD )
    {
    //如开发硬件单品，可删除以下宏定义内子设备相关的代码
    #ifdef IOT_DEVICE_IS_GATEWAY
        //连接云端成功后，应将网关下已经连接的子设备上报一次
        //有4个设备已连接到网关，这里添加4个子设备
        sub_dev_add("TEMP000001", "Thermometer", "01.01", 1);
        sub_dev_add("TEMP000002", "Thermometer", "01.01", 1);     
        sub_dev_add("FAN0000001", "electric fan", "01.01", 2);
        sub_dev_add("FAN0000002", "electric fan", "01.01", 2);
        iot_sub_dev_active( SUB_OPT_BIND_ONLINE, &seq);
        
        //连接云端成功后，应将设备最新的各数据点数据全部上报一次
        //上报子设备温湿度传感器TEMP000001的数据
        dp_up_add_int( "TEMP000001", DP_ID_TEMP, 26);
        dp_up_add_int( "TEMP000001", DP_ID_HUMIDITY, 35);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        //上报子设备温湿度传感器TEMP000002的数据
        dp_up_add_int( "TEMP000002", DP_ID_TEMP, 28);
        dp_up_add_int( "TEMP000002", DP_ID_HUMIDITY, 33);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        
        //上报子设备风扇FAN0000001的风速
        dp_up_add_enum( "FAN0000001", DP_ID_WIND_SPD, 3);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        //上报子设备风扇FAN0000002的风速
        dp_up_add_enum( "FAN0000002", DP_ID_WIND_SPD, 4);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
    #endif
        //上报网关自身的开关值
        dp_up_add_bool( NULL, DP_ID_DP_SWITCH, 1);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        
        //连接云端成功
        cloud_is_connected = 1;

        //允许APP绑定，60秒有效
        iot_status_set (DEV_STA_BIND_PERMIT,60);
    }
    else if( dev_status == DEV_STA_FACTORY_RESET )
    {
        //恢复出厂设置完成，请重启设备重新连接云端
        /* 示例
        system_reboot();
        */
    }
    else if(dev_status == DEV_STA_BIND_FAILED)
    {
        //用户绑定失败，允许绑定超时或者设备未联网
        printf("\r\nbind user timeout cb!\r\n");
    }
    else if(dev_status == DEV_STA_BIND_SUCCESS)
    {
        //用户绑定成功
        printf("\r\nbind user cb!\r\n");
    }
    else if(dev_status == DEV_STA_UNBIND)
    {
        //用户解绑成功
        printf("\r\n unbound user cb!\r\n");
    }
    else if(dev_status == DEV_STA_USER_SHARE_INC)
    {
        //分享用户增加
        printf("\r\n share user add cb!\r\n");
    } 
    else if(dev_status == DEV_STA_USER_SHARE_DEC)
    {
        //分享用户减少
        printf("\r\n share user delete cb!\r\n");
    }  
    else
    {
        //断开云端连接
        cloud_is_connected = 0;
    }
}

void iot_parse_timestamp( iot_u32 timestamp, s_time_t* st )
{
    printf("%s, timestamp:%d\r\n", __func__, timestamp);

    printf("time: %02d:%02d:%02d %02d:%02d:%02d week:%d\r\n", st->year, st->mon, st->day, st->hour, st->min, st->sec, st->week);
}

/*
 * 发送数据成功后，sdk自动调用下面这个回调函数
 */
void iot_data_cb ( iot_u32 data_seq, DATA_LIST_T * data_list )
{
    printf("iot_data_cb:%d. count:%d\r\n", data_seq, data_list->count);
    iot_u32 i = 0;
    
    for(i = 0 ; i < data_list->count ; i++)
    {
       printf("sub_id: %s err_type: %d \r\n",data_list->info[i].dev_id,data_list->info[i].info_type);
    }
}

//  第一部分结束  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  第二部分 [传输数据]  /////////////////////////////////////////////////////

//  各下发数据点处理函数，用户请按需自行实现  /////////////////////////////////

void dp_down_handle_switch (const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    iot_u32 seq = 0;
    iot_u8 dp_switch  = in_data[0];
    
    printf("dp_switch:%d\r\n", dp_switch);

    if(dp_switch == 0)
    {
        //关闭开关
    }
    else
    {
        //开启开关
    }
    //硬件操作完成后，应将更新后的状态上报一次
    dp_up_add_bool( sub_id, DP_ID_DP_SWITCH, dp_switch);
    //上报网关自身的开关值
    iot_upload_dps(DATA_ACT_NORMAL, &seq);
}

void dp_down_handle_temp(const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    int temp = STREAM_TO_UINT32_f(in_data, 0);
    printf("dp_temp:%d\r\n", temp);
}

void dp_down_handle_wind_spd (const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    iot_u32 seq;
    iot_u8 wind_spd = in_data[0];
    printf("wind_spd:%d\r\n", wind_spd);
    
    //如开发硬件单品，可删除以下宏定义内子设备相关的代码
    #ifdef IOT_DEVICE_IS_GATEWAY
        if(sub_id == NULL)
        {
            //网关自身数据
            printf("gateway wind_spd:%d\r\n", wind_spd);
        }
        else
        {
            //比较sub id，控制相应的子设备
            if ( strncmp( sub_id, "FAN0000001", 32 ) == 0 )
            {
                printf("get FAN0000001 cmd\r\n");
            }
            else if ( strncmp( sub_id, "FAN0000002", 32 ) == 0 )
            {
                printf("get FAN0000002 cmd\r\n");
            }
            else
            {
                printf("subid err\r\n");
            }
        }
    #endif
   
    //风速切换操作
    switch(wind_spd)
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;            
    case 4:
        break;
    case 5:
        break;
    default:break;
    }
    //硬件操作完成后，应将更新后的状态上报一次
    dp_up_add_enum( sub_id, DP_ID_WIND_SPD, wind_spd);
    
   iot_upload_dps(DATA_ACT_NORMAL, &seq);
}
void dp_down_handle_pm25(const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    float dp_pm25 = STREAM_TO_FLOAT32_f(in_data, 0);
    
    printf("dp_pm25:%f\r\n", dp_pm25);
}

void dp_down_handle_config (const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    printf("config:%s\r\n", in_data);
}

void dp_down_handle_bin (const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    int i = 0;
    
    printf("recv binary data[%d]:\r\n", in_len);
    
    for(i = 0; i < in_len;i++) 
    {
        printf("%02x ",in_data[i]);
    }
    printf("\r\n");
}

//  下发数据点处理函数结束  ////////////////////////////////////////////////////


//  下发数据点数据结构定义  ////////////////////////////////////////////////////

/*****************************************************************************
结构名称 : 下发数据点结构体数组
功能描述 : 用户请将需要处理的下发数据点，按数据点ID、数据点类型、
           数据点对应处理函数填写下面这个数组，并将处理函数实现
*****************************************************************************/
iot_download_dps_t iot_down_dps[] = 
{   /*  数据点ID     , 数据点类型  ,   处理函数               */
    { DP_ID_DP_SWITCH, DP_TYPE_BOOL,   dp_down_handle_switch   },
    { DP_ID_TEMP,      DP_TYPE_INT,    dp_down_handle_temp     },
    { DP_ID_PM25,      DP_TYPE_FLOAT,  dp_down_handle_pm25     },
    { DP_ID_WIND_SPD,  DP_TYPE_ENUM,   dp_down_handle_wind_spd },
    { DP_ID_CONFIG,    DP_TYPE_STRING, dp_down_handle_config   },
    { DP_ID_BIN,       DP_TYPE_BIN,    dp_down_handle_bin      }
};
/*****************************************************************************
结构名称 : DOWN_DPS_CNT整数
功能描述 : 下发数据点结构体数组中数据点数量，用户请勿更改
*****************************************************************************/
iot_u32 DOWN_DPS_CNT = sizeof(iot_down_dps)/sizeof(iot_download_dps_t);

//  下发数据点数据结构定义结束  /////////////////////////////////////////////////

void iot_rx_data_cb ( char sub_id[32], iot_u64 data_timestamp, iot_u8* data, iot_u32 data_len )
{
iot_u32 seq = 0;
    int i = 0, ret = 0;
    
    char c_sub_id[33] = {0};
    int timeout = 0;
    int temp = 0, wind = 0;
    int dp_sw = 0;
    iot_u8 local_str[128] = {0};

    printf("receive custom data[%d], timestamp:[%llu]\r\n", data_len, data_timestamp);

    //如开发硬件单品，可删除以下宏定义内子设备相关的代码
    #ifdef IOT_DEVICE_IS_GATEWAY
        if(sub_id == NULL)
        {
            //网关自身数据
            printf("gateway data :\r\n");
            //恢复出厂
            if(sscanf(data,"rst=%d", &timeout) == 1)
            {
                iot_status_set(DEV_STA_FACTORY_RESET, timeout);
            }
            //设备主动解绑
            else if(sscanf(data,"unb=%d", &timeout) == 1)
            {
                iot_status_set(DEV_STA_UNBIND, timeout);
            }
            //收到temp=13，则添加TEMP000013这个子设备上线
            else if(sscanf(data,"temp=%d", &temp) == 1)
            {
                printf("temp = %d online\r\n",temp);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "TEMP0000%02d", temp);
                
                sub_dev_add(c_sub_id, "Thermometer", "01.01", 0);
                iot_sub_dev_active(SUB_OPT_ONLINE, &seq);
            }
            //收到temp=13，则添加TEMP000013这个子设备绑定上线
            else if(sscanf(data,"b_temp=%d", &temp) == 1)
            {
                printf("temp = %d bind online\r\n",temp);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "TEMP0000%02d", temp);
                
                sub_dev_add(c_sub_id, "Thermometer", "01.01", 0);
                iot_sub_dev_active(SUB_OPT_BIND_ONLINE, &seq);
            }
            //收到wind=13，则添加FAN000013这个子设备上线
            else if(sscanf(data,"wind=%d", &wind) == 1)
            {
                printf("wind = %d online \r\n",wind);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "FAN00000%02d", wind);
                
                sub_dev_add(c_sub_id, "electric fan", "01.01", 1);
                iot_sub_dev_active(SUB_OPT_ONLINE, &seq);
            }      
            //收到wind=13，则添加FAN000013这个子设备绑定上线
            else if(sscanf(data,"b_wind=%d", &wind) == 1)
            {
                printf("wind = %d bind online \r\n",wind);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "FAN00000%02d", wind);
                
                sub_dev_add(c_sub_id, "electric fan", "01.01", 1);
                iot_sub_dev_active(SUB_OPT_BIND_ONLINE, &seq);
            }   
            else if(sscanf(data,"sw=%d", &dp_sw) == 1)
            {
                dp_up_add_bool( NULL, DP_ID_DP_SWITCH, dp_sw);                    
                iot_upload_dps(DATA_ACT_NORMAL, &seq);
                printf("send a normal data: dp_sw = %d \r\n",dp_sw);
            }
            else if(sscanf(data,"sw_fl=%d", &dp_sw) == 1)
            {
                dp_up_add_bool( NULL, DP_ID_DP_SWITCH, dp_sw);                    
                iot_upload_dps(DATA_ACT_FORBIT_LINKAGE, &seq);
                printf("send a forbit linkage data: dp_sw = %d \r\n",dp_sw);
            }
            else
            {
                printf("not supported for the moment!");
            }
        }
        else
        {
            printf("sub device [%s] data :\r\n", sub_id);
            //收到子设备的消息“offline”则将此子设备下线
            if(strncmp("offline", data, 6) == 0)
            {                
                printf("offline: sub_id = %s \r\n",sub_id);
                iot_sub_dev_inactive(sub_id, SUB_OPT_OFFLINE, &seq);
            }
            //收到子设备的消息“unb_offline”则将此子设备解绑下线
            else if(strncmp("u_offline", data, 6) == 0)       
            
            {                
                printf("unbind offline: sub_id = %s \r\n",sub_id);
                iot_sub_dev_inactive(sub_id, SUB_OPT_UNBIND_OFFLINE, &seq);
            }
        }
    #endif
    
    //先打印字符串，后以十六进制格式打印
    printf("str:%s\r\nhex:\r\n", data);
    for(i = 0; i < data_len; i++) 
    {
        printf("%02x ",data[i]);
    }
    printf("\r\n");
    
    //透传一条数据至手机端，云端不保存
    iot_tx_data(sub_id, &seq, data, data_len);
}

//  第二部分结束  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  第三部分 [远程升级]  /////////////////////////////////////////////////////
iot_u16 g_ota_crc = 0;
int g_ota_len = 0;
void iot_ota_info_cb( char name[16], char version[6], iot_u32 total_len, iot_u16 file_crc )
{
    printf("firmware [%s], version [%s], len [%d] crc [%d]will start upgrade\r\n", name, version, total_len, file_crc);
    g_ota_crc = file_crc;
    g_ota_len = total_len;

#if 1
    extern char mcu_ver[6];
    memcpy(mcu_ver, version, sizeof(mcu_ver));
#endif
}

void iot_ota_upgrade_cb ( void )
{
    printf("get ota cmd success,reboot right now\r\n");
    //处理收到的升级指令，需重启，运行新版本固件
    /* 示例
    system_reboot();
    */
}

#ifdef USE_PATCH            
extern patchs_list_t* g_patchs_list;
extern int g_patch_num ;
extern int g_patch_state;
#define PATCH_PERMIT_REQ 0
#define PATCH_UPGRADING 2

void iot_patchs_list_cb( iot_s32 count, patchs_list_t* patchs )
{
    int i = 0;
    
    printf("patchs count:%d\r\n", count);
    g_patch_num = count;
    g_patchs_list = patchs;
    g_patch_state = PATCH_PERMIT_REQ;
    for(i = 0; i < count; i++)
    {
        printf("patch [%d] :\r\n", i);
        printf("name:%s | version:%s | total_len:%d\r\n", g_patchs_list[i].name, g_patchs_list[i].version, g_patchs_list[i].total_len);
    }
}
#endif

iot_s32 iot_chunk_cb ( iot_u8 chunk_stat, iot_u32 chunk_offset, iot_u32 chunk_size, const iot_s8* chunk )
{
    printf("get %s chunk, offset = %d, size = %d\r\n", (IOT_CHUNK_TYPE(chunk_stat) == IOT_CHUNK_OTA) ? "OTA" : "PATCH", chunk_offset, chunk_size);
    // hex_dump("chunk", (iot_u8 *)chunk, chunk_size);
    
    //第一个数据包，准备升级空间
    if ( chunk_offset == 0 )
    {       
        //擦除flash，准备写入固件/补丁
        
       /*  固件示例
        *  flash_erase(start_address);
        */
        
       /*  补丁示例
        *  fp = fopen("/root/iot_patch_0102", "wb");
        */
       printf("first chunk\r\n");
    }
    else
    {   
        
    }
    //将固件/补丁数据块写入升级空间
        
    /*  固件示例
     *  flash_write( chunk_offset, chunk, chunk_size );
     */
     
    /* 补丁示例
     *  fwrite(chunk, chunk_size, 1, fp);
     */

    //最后一个数据包
    if ( IOT_CHUNK_IS_LAST(chunk_stat) ) 
    {       
        /*  固件示例，一般需要校验整个固件
        *  verify_firmware(start_address);
        */

       /*  补丁示例，一般需执行补丁升级流程
        *  fclose(fp);
        *  fp = NULL;
        *
        *  测试升级补丁
        *  ret = test_patch("/root/iot_patch_0102");
        *  将补丁升级状态上报至云端
        *  iot_patch_end("iot_patch", "0102", ret);
        */

        printf("get chunks over, wait to upgrade...\r\n");
        
#ifdef USE_PATCH    
        if(IOT_CHUNK_TYPE(chunk_stat) == IOT_CHUNK_PATCH)
            g_patch_state = PATCH_UPGRADING;  
#endif
        
    }
    
    return ACK_OK;
}

//  第三部分结束  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  第四部分 [设备信息]  /////////////////////////////////////////////////////

/*
 * 当设备接收到云端下发的设备信息数据，sdk自动调用此函数
 */
void iot_info_cb(INFO_TYPE_T info_type, void * info)
{
    int i = 0;
    
    user_info_t  master_info;
    share_info_t share_info;
    dev_info_t   dev_info;

    printf("\r\ninfo_type:%d\r\n", info_type);

    switch(info_type)
    {
        case INFO_TYPE_USER_MASTER:
            if(info == NULL){
                printf("no bound user!\r\n");
            }
            else {
                memcpy(&master_info, info, sizeof(user_info_t));
                printf("id:%s\r\n", master_info.id);
                printf("email:%s\r\n", master_info.email);
                printf("country_code:%s\r\n", master_info.country_code);
                printf("phone:%s\r\n", master_info.phone);
                printf("name:%s\r\n", master_info.name);
            }
            break;
        case INFO_TYPE_USER_SHARE:
            if(info == NULL){
                printf("no share user!\r\n");
            }
            else {
                memcpy(&share_info, info, sizeof(share_info_t));
                printf("count:%d\r\n", share_info.count);
                for(i = 0; i < share_info.count; i++)
                {
                    printf("share user %d\r\n", i);
                    printf("id:%s\r\n", share_info.user[i].id);
                    printf("email:%s\r\n", share_info.user[i].email);
                    printf("country_code:%s\r\n", share_info.user[i].country_code);
                    printf("phone:%s\r\n", share_info.user[i].phone);
                    printf("name:%s\r\n", share_info.user[i].name);
                    printf("\r\n");
                }
            }
            break;
        case INFO_TYPE_DEVICE:
            memcpy(&dev_info, info, sizeof(dev_info_t));
            printf("is_bind:%d,sdk_version:%s\r\n", dev_info.is_bind,dev_info.sdk_ver);
            break;
        default:
            printf("info_type is error\r\n");
    }
}

//  第四部分结束  ////////////////////////////////////////////////////////////


/*
 * 当子设备状态发生改变时，sdk自动调用此函数
 */
void iot_sub_status_cb( SUB_STATUS_T sub_status, iot_u32 timestamp, DATA_LIST_T * p_sub_list , iot_u32 seq)
{
    printf("sub device state:%d, list_count: %d, ts:%d, seq = %d\r\n", sub_status, p_sub_list->count, timestamp, seq);
    int i = 0;    

    if( sub_status == SUB_STA_ONLINE )
    {  
        for( i = 0; i < p_sub_list->count; i++)
        {
             printf("sub_id: %s online err: %d\r\n", p_sub_list->info[i].dev_id, p_sub_list->info[i].info_type);           
        } 
    }
    else if( sub_status == SUB_STA_BIND_ONLINE )
    {    
        for( i = 0; i < p_sub_list->count; i++)
        {
             printf("sub_id: %s bind_online err: %d\r\n", p_sub_list->info[i].dev_id, p_sub_list->info[i].info_type);           
        }
    }
    else if( sub_status == SUB_STA_OFFLINE )
    {        
        for( i = 0; i < p_sub_list->count; i++)
        {
             printf("sub_id: %s offline err: %d\r\n", p_sub_list->info[i].dev_id, p_sub_list->info[i].info_type);           
        } 
    }
    else if( sub_status == SUB_STA_UNB_OFFLINE )
    {    
        for( i = 0; i < p_sub_list->count; i++)
        {
             printf("sub_id: %s unb_offline err: %d\r\n", p_sub_list->info[i].dev_id, p_sub_list->info[i].info_type);           
        } 

    }
    else if( sub_status == SUB_STA_PASSIVE_UNBIND )
    {  
        for(i = 0; i < p_sub_list->count; i++)
        {
            printf("sub_id : %s forced unbind \r\n", p_sub_list->info[i].dev_id);           
        }
    }     
}
/*##########################################################################*/

//end iot_interface.c

