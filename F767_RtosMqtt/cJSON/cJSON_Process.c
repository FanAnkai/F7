#include "cJSON_Process.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"




cJSON* cJSON_Data_Init(void)
{
    cJSON* cJSON_Root = NULL;    //json根节点

    cJSON_Root = cJSON_CreateObject();   //创建项目
    if(NULL == cJSON_Root)
    {
        return NULL;
    }
    cJSON_AddStringToObject(cJSON_Root, NAME, DEFAULT_NAME);  //*添加元素  键值对
    cJSON_AddNumberToObject(cJSON_Root, TEMP_NUM, DEFAULT_TEMP_NUM);
    cJSON_AddNumberToObject(cJSON_Root, HUM_NUM, DEFAULT_HUM_NUM);

    char* p = cJSON_Print(cJSON_Root);  //p 指向的字符串是json格式的

    PRINT_DEBUG("%s\n", p);

    vPortFree(p);
    p = NULL;

    return cJSON_Root;

}
uint8_t cJSON_Update(const cJSON * const object, const char * const string, void *d)
{
    cJSON* node = NULL;    //json根节点
    node = cJSON_GetObjectItem(object, string);
    if(node == NULL)
        return NULL;
    if(cJSON_IsBool(node))
    {
        int *b = (int*)d;
        cJSON_GetObjectItem(object, string)->type = *b ? cJSON_True : cJSON_False;
        return 1;
    }
    else if(cJSON_IsString(node))
    {
        cJSON_GetObjectItem(object, string)->valuestring = (char*)d;
        return 1;
    }
    else if(cJSON_IsNumber(node))
    {
        double *num = (double*)d;
        cJSON_GetObjectItem(object, string)->valuedouble = (double) * num;
        return 1;
    }
    else
        return 1;
}

void Proscess(void* data)
{
    PRINT_DEBUG("开始解析JSON数据");
    cJSON *root, *json_name, *json_temp_num, *json_hum_num;
    root = cJSON_Parse((char*)data); //解析成json形式
    //LightLuminance
    json_name = cJSON_GetObjectItem( root , NAME);  //获取键值内容
    json_temp_num = cJSON_GetObjectItem( root , TEMP_NUM );
    json_hum_num = cJSON_GetObjectItem( root , HUM_NUM );

    PRINT_DEBUG("name:%s\n temp_num:%f\n hum_num:%f\n",
                json_name->valuestring,
                json_temp_num->valuedouble,
                json_hum_num->valuedouble);

    cJSON_Delete(root);  //释放内存
}








