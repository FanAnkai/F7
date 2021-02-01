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

//  MCU�ض����ֿ�ʼ  ////////////////////////////////////////////////////////////

//MCU ��������Ļظ����ж������Ƿ�ɹ�
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

//  MCU�ض����ֽ���  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  ��һ���� [ϵͳ����]  /////////////////////////////////////////////////////

/*
 * ��ӡ��־������������ʵ�֣��ɸ�����Ҫ�ض��򵽴��ڡ���Ļ���ļ���λ��
 */
void iot_print(const char *str)
{
    printf("%s", str);
}

/*
 * ���豸����״̬�����ı�ʱ��sdk�Զ����ô˺���
 */
void iot_status_cb( DEV_STATUS_T dev_status, iot_u32 timestamp )
{
    iot_u32 seq = 0;
    printf("device state:%d, ts:%d\r\n", dev_status, timestamp);

    if( dev_status == DEV_STA_CONNECTED_CLOUD )
    {
    //�翪��Ӳ����Ʒ����ɾ�����º궨�������豸��صĴ���
    #ifdef IOT_DEVICE_IS_GATEWAY
        //�����ƶ˳ɹ���Ӧ���������Ѿ����ӵ����豸�ϱ�һ��
        //��4���豸�����ӵ����أ��������4�����豸
        sub_dev_add("TEMP000001", "Thermometer", "01.01", 1);
        sub_dev_add("TEMP000002", "Thermometer", "01.01", 1);     
        sub_dev_add("FAN0000001", "electric fan", "01.01", 2);
        sub_dev_add("FAN0000002", "electric fan", "01.01", 2);
        iot_sub_dev_active( SUB_OPT_BIND_ONLINE, &seq);
        
        //�����ƶ˳ɹ���Ӧ���豸���µĸ����ݵ�����ȫ���ϱ�һ��
        //�ϱ����豸��ʪ�ȴ�����TEMP000001������
        dp_up_add_int( "TEMP000001", DP_ID_TEMP, 26);
        dp_up_add_int( "TEMP000001", DP_ID_HUMIDITY, 35);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        //�ϱ����豸��ʪ�ȴ�����TEMP000002������
        dp_up_add_int( "TEMP000002", DP_ID_TEMP, 28);
        dp_up_add_int( "TEMP000002", DP_ID_HUMIDITY, 33);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        
        //�ϱ����豸����FAN0000001�ķ���
        dp_up_add_enum( "FAN0000001", DP_ID_WIND_SPD, 3);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        //�ϱ����豸����FAN0000002�ķ���
        dp_up_add_enum( "FAN0000002", DP_ID_WIND_SPD, 4);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
    #endif
        //�ϱ���������Ŀ���ֵ
        dp_up_add_bool( NULL, DP_ID_DP_SWITCH, 1);
        iot_upload_dps(DATA_ACT_NORMAL, &seq);
        
        //�����ƶ˳ɹ�
        cloud_is_connected = 1;

        //����APP�󶨣�60����Ч
        iot_status_set (DEV_STA_BIND_PERMIT,60);
    }
    else if( dev_status == DEV_STA_FACTORY_RESET )
    {
        //�ָ�����������ɣ��������豸���������ƶ�
        /* ʾ��
        system_reboot();
        */
    }
    else if(dev_status == DEV_STA_BIND_FAILED)
    {
        //�û���ʧ�ܣ�����󶨳�ʱ�����豸δ����
        printf("\r\nbind user timeout cb!\r\n");
    }
    else if(dev_status == DEV_STA_BIND_SUCCESS)
    {
        //�û��󶨳ɹ�
        printf("\r\nbind user cb!\r\n");
    }
    else if(dev_status == DEV_STA_UNBIND)
    {
        //�û����ɹ�
        printf("\r\n unbound user cb!\r\n");
    }
    else if(dev_status == DEV_STA_USER_SHARE_INC)
    {
        //�����û�����
        printf("\r\n share user add cb!\r\n");
    } 
    else if(dev_status == DEV_STA_USER_SHARE_DEC)
    {
        //�����û�����
        printf("\r\n share user delete cb!\r\n");
    }  
    else
    {
        //�Ͽ��ƶ�����
        cloud_is_connected = 0;
    }
}

void iot_parse_timestamp( iot_u32 timestamp, s_time_t* st )
{
    printf("%s, timestamp:%d\r\n", __func__, timestamp);

    printf("time: %02d:%02d:%02d %02d:%02d:%02d week:%d\r\n", st->year, st->mon, st->day, st->hour, st->min, st->sec, st->week);
}

/*
 * �������ݳɹ���sdk�Զ�������������ص�����
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

//  ��һ���ֽ���  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  �ڶ����� [��������]  /////////////////////////////////////////////////////

//  ���·����ݵ㴦�������û��밴������ʵ��  /////////////////////////////////

void dp_down_handle_switch (const char sub_id[32], iot_u8* in_data, iot_u32 in_len )
{
    iot_u32 seq = 0;
    iot_u8 dp_switch  = in_data[0];
    
    printf("dp_switch:%d\r\n", dp_switch);

    if(dp_switch == 0)
    {
        //�رտ���
    }
    else
    {
        //��������
    }
    //Ӳ��������ɺ�Ӧ�����º��״̬�ϱ�һ��
    dp_up_add_bool( sub_id, DP_ID_DP_SWITCH, dp_switch);
    //�ϱ���������Ŀ���ֵ
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
    
    //�翪��Ӳ����Ʒ����ɾ�����º궨�������豸��صĴ���
    #ifdef IOT_DEVICE_IS_GATEWAY
        if(sub_id == NULL)
        {
            //������������
            printf("gateway wind_spd:%d\r\n", wind_spd);
        }
        else
        {
            //�Ƚ�sub id��������Ӧ�����豸
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
   
    //�����л�����
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
    //Ӳ��������ɺ�Ӧ�����º��״̬�ϱ�һ��
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

//  �·����ݵ㴦��������  ////////////////////////////////////////////////////


//  �·����ݵ����ݽṹ����  ////////////////////////////////////////////////////

/*****************************************************************************
�ṹ���� : �·����ݵ�ṹ������
�������� : �û��뽫��Ҫ������·����ݵ㣬�����ݵ�ID�����ݵ����͡�
           ���ݵ��Ӧ��������д����������飬����������ʵ��
*****************************************************************************/
iot_download_dps_t iot_down_dps[] = 
{   /*  ���ݵ�ID     , ���ݵ�����  ,   ������               */
    { DP_ID_DP_SWITCH, DP_TYPE_BOOL,   dp_down_handle_switch   },
    { DP_ID_TEMP,      DP_TYPE_INT,    dp_down_handle_temp     },
    { DP_ID_PM25,      DP_TYPE_FLOAT,  dp_down_handle_pm25     },
    { DP_ID_WIND_SPD,  DP_TYPE_ENUM,   dp_down_handle_wind_spd },
    { DP_ID_CONFIG,    DP_TYPE_STRING, dp_down_handle_config   },
    { DP_ID_BIN,       DP_TYPE_BIN,    dp_down_handle_bin      }
};
/*****************************************************************************
�ṹ���� : DOWN_DPS_CNT����
�������� : �·����ݵ�ṹ�����������ݵ��������û��������
*****************************************************************************/
iot_u32 DOWN_DPS_CNT = sizeof(iot_down_dps)/sizeof(iot_download_dps_t);

//  �·����ݵ����ݽṹ�������  /////////////////////////////////////////////////

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

    //�翪��Ӳ����Ʒ����ɾ�����º궨�������豸��صĴ���
    #ifdef IOT_DEVICE_IS_GATEWAY
        if(sub_id == NULL)
        {
            //������������
            printf("gateway data :\r\n");
            //�ָ�����
            if(sscanf(data,"rst=%d", &timeout) == 1)
            {
                iot_status_set(DEV_STA_FACTORY_RESET, timeout);
            }
            //�豸�������
            else if(sscanf(data,"unb=%d", &timeout) == 1)
            {
                iot_status_set(DEV_STA_UNBIND, timeout);
            }
            //�յ�temp=13�������TEMP000013������豸����
            else if(sscanf(data,"temp=%d", &temp) == 1)
            {
                printf("temp = %d online\r\n",temp);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "TEMP0000%02d", temp);
                
                sub_dev_add(c_sub_id, "Thermometer", "01.01", 0);
                iot_sub_dev_active(SUB_OPT_ONLINE, &seq);
            }
            //�յ�temp=13�������TEMP000013������豸������
            else if(sscanf(data,"b_temp=%d", &temp) == 1)
            {
                printf("temp = %d bind online\r\n",temp);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "TEMP0000%02d", temp);
                
                sub_dev_add(c_sub_id, "Thermometer", "01.01", 0);
                iot_sub_dev_active(SUB_OPT_BIND_ONLINE, &seq);
            }
            //�յ�wind=13�������FAN000013������豸����
            else if(sscanf(data,"wind=%d", &wind) == 1)
            {
                printf("wind = %d online \r\n",wind);
                memset(c_sub_id, 0, 33);
                sprintf(c_sub_id, "FAN00000%02d", wind);
                
                sub_dev_add(c_sub_id, "electric fan", "01.01", 1);
                iot_sub_dev_active(SUB_OPT_ONLINE, &seq);
            }      
            //�յ�wind=13�������FAN000013������豸������
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
            //�յ����豸����Ϣ��offline���򽫴����豸����
            if(strncmp("offline", data, 6) == 0)
            {                
                printf("offline: sub_id = %s \r\n",sub_id);
                iot_sub_dev_inactive(sub_id, SUB_OPT_OFFLINE, &seq);
            }
            //�յ����豸����Ϣ��unb_offline���򽫴����豸�������
            else if(strncmp("u_offline", data, 6) == 0)       
            
            {                
                printf("unbind offline: sub_id = %s \r\n",sub_id);
                iot_sub_dev_inactive(sub_id, SUB_OPT_UNBIND_OFFLINE, &seq);
            }
        }
    #endif
    
    //�ȴ�ӡ�ַ���������ʮ�����Ƹ�ʽ��ӡ
    printf("str:%s\r\nhex:\r\n", data);
    for(i = 0; i < data_len; i++) 
    {
        printf("%02x ",data[i]);
    }
    printf("\r\n");
    
    //͸��һ���������ֻ��ˣ��ƶ˲�����
    iot_tx_data(sub_id, &seq, data, data_len);
}

//  �ڶ����ֽ���  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  �������� [Զ������]  /////////////////////////////////////////////////////
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
    //�����յ�������ָ��������������°汾�̼�
    /* ʾ��
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
    
    //��һ�����ݰ���׼�������ռ�
    if ( chunk_offset == 0 )
    {       
        //����flash��׼��д��̼�/����
        
       /*  �̼�ʾ��
        *  flash_erase(start_address);
        */
        
       /*  ����ʾ��
        *  fp = fopen("/root/iot_patch_0102", "wb");
        */
       printf("first chunk\r\n");
    }
    else
    {   
        
    }
    //���̼�/�������ݿ�д�������ռ�
        
    /*  �̼�ʾ��
     *  flash_write( chunk_offset, chunk, chunk_size );
     */
     
    /* ����ʾ��
     *  fwrite(chunk, chunk_size, 1, fp);
     */

    //���һ�����ݰ�
    if ( IOT_CHUNK_IS_LAST(chunk_stat) ) 
    {       
        /*  �̼�ʾ����һ����ҪУ�������̼�
        *  verify_firmware(start_address);
        */

       /*  ����ʾ����һ����ִ�в�����������
        *  fclose(fp);
        *  fp = NULL;
        *
        *  ������������
        *  ret = test_patch("/root/iot_patch_0102");
        *  ����������״̬�ϱ����ƶ�
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

//  �������ֽ���  ////////////////////////////////////////////////////////////


/*##########################################################################*/


//  ���Ĳ��� [�豸��Ϣ]  /////////////////////////////////////////////////////

/*
 * ���豸���յ��ƶ��·����豸��Ϣ���ݣ�sdk�Զ����ô˺���
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

//  ���Ĳ��ֽ���  ////////////////////////////////////////////////////////////


/*
 * �����豸״̬�����ı�ʱ��sdk�Զ����ô˺���
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

