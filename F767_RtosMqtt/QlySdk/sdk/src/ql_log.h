#ifndef __QL_LOG__
#define __QL_LOG__
#include "ql_include.h"

//#define DEBUG_DATA_OPEN
#define G_LOG_STR_LEN 512

#ifdef DEBUG_DATA_OPEN
    #ifndef ql_printf
        #define ql_printf   printf
    #endif
#endif

//#define DEBUG_WIFI_OTA_OPEN

extern char* get_time_str(void);

extern char g_log_str[];
extern char g_cld_log_str[];
#define _STRCAT(x,y) (x y)
#define STRCAT(x,y) _STRCAT(x,y)

#define _STRCAT_TR(x,y,z) (x y z)
#define STRCAT_TR(x,y,z) _STRCAT_TR(x,y,z)

typedef enum LOG_ON_SW {
    LOG_ONLINE_SW_CLOSE     = 0,  //关闭log上报
    LOG_ONLINE_SW_NORMAL    = 1,  //只上报info和warn类型log
    LOG_ONLINE_SW_ERR       = 2,  //只上报err类型log
    LOG_ONLINE_SW_ALL       = 3,  //log全部上报
}LOG_ON_SW_T;

typedef enum LOG_TYPE {
    LOG_TYPE_INFO           = 1,  //普通信息
    LOG_TYPE_WARN           = 2,  //警告信息
    LOG_TYPE_ERR            = 3,  //错误信息
}LOG_TYPE_T;

/*普通信息类型*/
typedef enum INFO_EVENT {
    INFO_EVENT_DEV          = 1,  //设备信息
    INFO_EVENT_PERMIT_BIND  = 2,  //允许绑定
    INFO_EVENT_OTA_TS = 3,  //OTA检测时间
}INFO_EVENT_T;

/*警告信息类型*/
typedef enum WARN_EVENT {
    WARN_EVENT_DISCONN          = 1,  //长连接断开
    WARN_EVENT_CONN_EXC         = 2,  //网络连接异常
    WARN_EVENT_UNB_EXC          = 3,  //解绑异常
    WARN_EVENT_P_BIND_EXC       = 4,  //允许绑定异常
    WARN_EVENT_RST_EXC          = 5,  //恢复出厂异常    
}WARN_EVENT_T;

/*错误信息类型*/
typedef enum ERR_EVENT {
    ERR_EVENT_NULL          = 0,  //不上报错误
    ERR_EVENT_STATU         = 1,  //状态错误
    ERR_EVENT_INTERFACE     = 2,  //接口错误
    ERR_EVENT_RX_DATA       = 3,  //接收数据错误
    ERR_EVENT_TX_DATA       = 4,  //发送数据错误
    ERR_EVENT_LOCAL_DATA    = 5,  //本地数据错误
    ERR_EVENT_LOG_IF        = 6,  //LOG接口错误
}ERR_EVENT_T;


#if (__SDK_PLATFORM__ == 0x02 || __SDK_PLATFORM__ == 0x03 ) /*esp8266 freeRtos*/
    #define LOG_INFO_FMT "[IOT-INFO-%s-%d] "
    #define LOG_WARN_FMT "[IOT-WARN-%s-%d] "
    #define LOG_ERR_FMT  "[IOT-ERR-%d-%s-%d] "

    //  !!!the length of log string must be not exceeding 100 bytes.
    #ifdef DEBUG_VERSION

        #define ql_log_info( fmt, ... ) do{\
            ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT(LOG_INFO_FMT, fmt), get_time_str(), system_get_free_heap_size(), ##__VA_ARGS__);\
            iot_print(g_log_str);   \
        } while(0)

        #define ql_log_warn( fmt, ... ) do{\
            ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT(LOG_WARN_FMT, fmt), get_time_str(), system_get_free_heap_size(), ##__VA_ARGS__);\
            iot_print(g_log_str);   \
        } while(0)
    #else
        #define ql_log_info( fmt, ... )
        #define ql_log_warn( fmt, ... )
    #endif

    #ifdef LOCAL_SAVE
extern ql_mutex_t* g_local_config_mutex;
        /*注意:参数fmt不得超过20个字符*/
        #define ql_log_err(event, fmt, ... ) do{\
                ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT_TR(LOG_ERR_FMT,fmt,"\r\n"),event, get_time_str(), system_get_free_heap_size(), ##__VA_ARGS__);\
                iot_print(g_log_str);   \
                if(event != 0 && g_local_config_mutex != NULL)      \
                {\
                    memset(g_cld_log_str, 0x00, 26);\
                    ql_snprintf(g_cld_log_str, 26, fmt, ##__VA_ARGS__);\
                    ql_local_log_to_cloud(LOG_TYPE_ERR,event,(void *)g_cld_log_str, get_current_time());\
                }\
            } while(0)
    #else
        /*注意:参数fmt不得超过20个字符*/
        #define ql_log_err(event, fmt, ... ) do{\
                ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT_TR(LOG_ERR_FMT,fmt,"\r\n"),event, get_time_str(), system_get_free_heap_size(), ##__VA_ARGS__);\
                iot_print(g_log_str);   \
            } while(0)
    #endif
#else

    #define LOG_INFO_FMT "[IOT-INFO-%s] "
    #define LOG_WARN_FMT "[IOT-WARN-%s] "
    #define LOG_ERR_FMT  "[IOT-ERR-%d-%s] "

    //  !!!the length of log string must be not exceeding 100 bytes.
    #ifdef DEBUG_VERSION

        #define ql_log_info( fmt, ... ) do{\
            ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT(LOG_INFO_FMT, fmt), get_time_str(), ##__VA_ARGS__);\
            iot_print(g_log_str);   \
        } while(0)

        #define ql_log_warn( fmt, ... ) do{\
            ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT(LOG_WARN_FMT, fmt), get_time_str(), ##__VA_ARGS__);\
            iot_print(g_log_str);   \
        } while(0)

    #else

        #define ql_log_info( fmt, ... )
        #define ql_log_warn( fmt, ... )

    #endif

    #ifdef LOCAL_SAVE
        /*注意:参数fmt不得超过20个字符*/
        #define ql_log_err(event, fmt, ... ) do{\
                ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT_TR(LOG_ERR_FMT,fmt,"\r\n"),event, get_time_str(), ##__VA_ARGS__);\
                iot_print(g_log_str);   \
                if(event != 0)      \
                {\
                    memset(g_cld_log_str, 0x00, 26);\
                    ql_snprintf(g_cld_log_str, 26, fmt, ##__VA_ARGS__);\
                    ql_local_log_to_cloud(LOG_TYPE_ERR,event,(void *)g_cld_log_str, get_current_time());\
                }\
            } while(0)
    #else
        /*注意:参数fmt不得超过20个字符*/
        #define ql_log_err(event, fmt, ... ) do{\
                ql_snprintf(g_log_str, G_LOG_STR_LEN, STRCAT_TR(LOG_ERR_FMT,fmt,"\r\n"),event, get_time_str(), ##__VA_ARGS__);\
                iot_print(g_log_str);   \
            } while(0)
    #endif
#endif

#endif

