

#include "iot_interface.h"
#include "protocol.h"
#include "mcu_if.h"
#include "spec_opcode.h"

extern int cloud_is_connected;

void test_data_up()
{
    static iot_u32 Temperature = 0;
    static iot_u8 Switch = 0;
    static iot_u8 Wind_spd = 0;
    iot_u8* Config[] = {"CONFIG-1", "CONFIG-2", "CONFIG-3"};
    static iot_f32 Pm25 = 0.1;
    iot_u8* Fault[] = {"FAULT-1", "FAULT-2", "FAULT-3"};
    iot_u8 Bin[][3] = {{0x11, 0x22, 0x33}, {0x44, 0x55, 0x66}, {0x77, 0x88, 0x99}};

    iot_u32 data_seq = 0;

    static iot_u8 i = 0;
    i ++;
    if(3 == i)
    {
        i = 0;
    }

    /* Wind_spd : 0 ~ 5 */
    Wind_spd ++;
    if(6 == Wind_spd)
    {
        Wind_spd = 0;
    }

    /* Switch : 0 , 1 */
    if(0 == Switch)
    {
        Switch = 1;
    }
    else
    {
        Switch = 0;
    }

    dp_up_add_int(NULL, DP_ID_TEMP, Temperature ++);
    
    dp_up_add_bool(NULL, DP_ID_DP_SWITCH, Switch);
    
    dp_up_add_enum(NULL, DP_ID_WIND_SPD, Wind_spd);
    
    dp_up_add_string(NULL, DP_ID_CONFIG, Config[i], strlen(Config[i]));
    
    dp_up_add_float(NULL, DP_ID_PM25, Pm25 += 0.1);
    
    dp_up_add_fault(NULL, DP_ID_FAULT, Fault[i], 7);
    
    dp_up_add_binary(NULL, DP_ID_BIN, Bin[i], 3);

    iot_upload_dps(DATA_ACT_NORMAL, &data_seq);

    printf("\r\nstart test data up, Seq:%d\r\n", data_seq);
}

void test_custom_data_up()
{
    iot_u32 Seq = 0;
    static int32_t i = 0;
    iot_u8 tmp = 0;
    iot_u8 TestBuf[] = "this is device push message";

    iot_tx_data(NULL, &Seq, TestBuf, sizeof(TestBuf));
    printf("\r\nstart test custom data up, Seq:%d\r\n", Seq);
}

#ifdef USE_PATCH          
#define PATCH_PERMIT_REQ 0  /*patch ????????*/
#define PATCH_DOWNLOADING 1 /*patch ????????*/
#define PATCH_UPGRADING 2     /*patch ????????*/

patchs_list_t* g_patchs_list;
int g_patch_num = 0;/*?????????????*/
int g_patch_state = 0; /*?????????*/

int test_patch(char *p)
{
    return DEV_STA_PATCH_SUCCESS;
}
#endif

void test_patch_req()
{
#ifdef USE_PATCH  
    int ret = 0;
    switch(g_patch_state)
    {
        case PATCH_PERMIT_REQ:
            if(g_patchs_list && g_patch_num > 0)
            {     
                g_patch_num--;
                printf("patch_req name[%d]:%s,ver:%s \r\n", g_patch_num, g_patchs_list[g_patch_num].name, g_patchs_list[g_patch_num].version);
                if(-1 == iot_patch_req(g_patchs_list[g_patch_num].name,g_patchs_list[g_patch_num].version))
                    printf("patch name: %s ver:%s request err\r\n",g_patchs_list[g_patch_num].name,g_patchs_list[g_patch_num].version);
                else
                    g_patch_state = PATCH_DOWNLOADING;   
            }
            break;
        case PATCH_UPGRADING:
            /*???????????*/
            ret = test_patch(g_patchs_list[g_patch_num].name);
            /*?????????????*/
            iot_patch_end(g_patchs_list[g_patch_num].name,g_patchs_list[g_patch_num].version,ret);
            g_patch_state = PATCH_PERMIT_REQ;
            break;
        default:
            break;
    }
#endif
}

void test_get_info()
{
    static INFO_TYPE_T get_info_status = INFO_TYPE_USER_MASTER;

    iot_get_info(get_info_status);

    switch (get_info_status)
    {
    case INFO_TYPE_USER_MASTER:
        get_info_status = INFO_TYPE_USER_SHARE;
        break;
    case INFO_TYPE_USER_SHARE:
        get_info_status = INFO_TYPE_DEVICE;
        break;
    case INFO_TYPE_DEVICE:
        get_info_status = INFO_TYPE_USER_MASTER;
        break;
    default:
        break;
    }
}

void test_subdev_active()
{
    iot_u32 data_seq = 0;
    
    sub_dev_add("TEMP000001", "Thermometer", "01.01", 1);
    sub_dev_add("TEMP000002", "Thermometer", "01.01", 1);
    sub_dev_add("FAN0000001", "electric fan", "01.01", 2);
    sub_dev_add("FAN0000002", "electric fan", "01.01", 2);
    iot_sub_dev_active(SUB_OPT_BIND_ONLINE, &data_seq);
}

void test_subdev_inactive()
{
    iot_u32 data_seq1 = 0;
    iot_u32 data_seq2 = 0;
    iot_u32 data_seq3 = 0;
    iot_u32 data_seq4 = 0;
    
    iot_sub_dev_inactive("TEMP000001", SUB_OPT_OFFLINE, &data_seq1);
    iot_sub_dev_inactive("TEMP000002", SUB_OPT_UNBIND_OFFLINE, &data_seq2);
    iot_sub_dev_inactive("FAN0000001", SUB_OPT_OFFLINE, &data_seq3);
    iot_sub_dev_inactive("FAN0000002", SUB_OPT_UNBIND_OFFLINE, &data_seq4);
}

void test_get_module_info()
{
    static iot_u8 module_info_type = MODULE_INFO_TYPE_ROUTER;

    iot_module_info_get(module_info_type);

    switch (module_info_type)
    {
    case MODULE_INFO_TYPE_ROUTER:
        module_info_type = MODULE_INFO_TYPE_GETIP;
        break;
    case MODULE_INFO_TYPE_GETIP:
        module_info_type = MODULE_INFO_TYPE_ROUTER;
        break;
    default:
        break;
    }
}


extern int cloud_is_connected;
char mcu_ver[6] = "01.01";
static struct iot_context iot_ctx = 
{\
    1003548929, \
    {0x76, 0xdc, 0xa2, 0x37, 0x78, 0x47, 0x79, 0x34, 0x22, 0x6f, 0x50, 0xa3, 0x25, 0x23, 0x70, 0x61}, \
    mcu_ver, \
    2048, \
    2048, \
    "dispatch.qinglianyun.com",
    8990
};

void task_wifi_entry(void *argument)
{
	int count_m = 0;
    int test_type = 0;
	
    mcu_protocol_init();
	
	printf("init!\r\n");

    osDelay(5000);

    iot_start(&iot_ctx);

#if 1
//	iot_module_set_factroy(120);
	iot_module_set_smartconfig(120);
//	iot_module_set_ap(120, "iot_ap_test", NULL);
//	iot_module_set_reset(120);
//	iot_status_set(DEV_STA_FACTORY_RESET, 120);
#endif

    count_m = 0;
    while(1)
    {
        serial_rx_handle();
        
        if(1 == cloud_is_connected)
        {
            if((++count_m)%500 == 0) /* timer */
            {
#if 1
                if(0 == test_type) /* dp test */
                {
                    test_data_up();
                    test_type = 1;
                }
                else if(1 == test_type) /* push test */
                {
                    test_custom_data_up();
                    test_type = 0;
                }
#endif        
				 iot_get_onlinetime();
				 test_patch_req();
				 test_get_info();
				 test_subdev_active();
				 test_subdev_inactive();
				 test_get_module_info();
            }
        }
		
		osDelay(50);
    }

}



