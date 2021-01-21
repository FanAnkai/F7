#ifndef __MALLOC_H
#define __MALLOC_H
#include "stm32f7xx.h"
#include "lwipopts.h"


/************************ DHT11 �������Ͷ���******************************/
typedef struct
{
//    uint8_t  humi_high8bit;     //ԭʼ���ݣ�ʪ�ȸ�8λ
//    uint8_t  humi_low8bit;      //ԭʼ���ݣ�ʪ�ȵ�8λ
//    uint8_t  temp_high8bit;     //ԭʼ���ݣ��¶ȸ�8λ
//    uint8_t  temp_low8bit;      //ԭʼ���ݣ��¶ȸ�8λ
//    uint8_t  check_sum;         //У���
    double    humidity;         //ʵ��ʪ��
    double    temperature;      //ʵ���¶�

} DHT11_Data_TypeDef;



#define   MSG_MAX_LEN     500
#define   MSG_TOPIC_LEN   50
#define   KEEPLIVE_TIME   50
#define   MQTT_VERSION    4

// LWIP_MQTT_TEST
// a1uyBz1glUb
// fUnfCXvFGx3msAcT
//2da13a4bff37e121576680ef62049292



#if LWIP_DNS
#define     HOST_NAME       "a1MZHjK2SCF.iot-as-mqtt.cn-shanghai.aliyuncs.com"  //����������
#define     HOST_PORT       1883        //������TCP���ӣ��˿ڱ�����1883
#else
#define     HOST_NAME       "139.196.135.135"   //������IP��ַ
#define     HOST_PORT     1883      //������TCP���ӣ��˿ڱ�����1883
#endif

#define DEVICE_NAME     "F767_LWIP"
#define PRODUCT_KEY     "a1uyBz1glUb"
#define USER_CLIENT_ID  "123456"


#define     CLIENT_ID     "123456|securemode=3,signmethod=hmacsha1|"       //
#define     USER_NAME     "F767_LWIP&a1uyBz1glUb"                         //�û�
#define     PASSWORD      "53427208084CCBCF7213B7BF02E6AC101DF97CC5"      //��Կ

#define     TOPIC0         "/a1uyBz1glUb/" DEVICE_NAME "/user/FYF_TEST"      //���ĵ�����
#define     TOPIC1         "/sys/a1uyBz1glUb/" DEVICE_NAME "/thing/service/property/set"
#define     TOPIC2         "/sys/a1uyBz1glUb/" DEVICE_NAME "/thing/event/property/post_reply"

#define     TOPIC3         "/sys/a1uyBz1glUb/" DEVICE_NAME "/thing/event/property/post"

extern int mqtt_connect_flag;

//*****�յ����ĵ���Ϣ��******
//MQTT>>��Ϣ������QoS0
//MQTT>>��Ϣ���⣺/sys/a1uyBz1glUb/F767_LWIP/thing/service/property/
//MQTT>>��Ϣ���ݣ�{"method":"thing.service.property.set","id":"765854702","params":{"LightLuminance":100},"version":"1.0.0"}


//*****�յ����ĵ���Ϣ��******
//MQTT>>��Ϣ������QoS0
//MQTT>>��Ϣ���⣺/a1uyBz1glUb/F767_LWIP/user/FYF_TEST
//MQTT>>��Ϣ���ݣ�123
//MQTT>>��Ϣ���ȣ�3


enum QoS
{ QOS0 = 0,
  QOS1,
  QOS2
};

enum MQTT_Connect
{
    Connect_OK = 0,
    Connect_NOK,
    Connect_NOTACK
};

//���ݽ����ṹ��
typedef struct __MQTTMessage
{
    uint32_t qos;
    uint8_t retained;
    uint8_t dup;
    uint16_t id;
    uint8_t type;
    void *payload;
    int32_t payloadlen;
} MQTTMessage;

//�û�������Ϣ�ṹ��
typedef struct __MQTT_MSG
{
    uint8_t  msgqos;                 //��Ϣ����
    uint8_t  msg[MSG_MAX_LEN];       //��Ϣ
    uint32_t msglenth;               //��Ϣ����
    uint8_t  topic[MSG_TOPIC_LEN];   //����
    uint16_t packetid;               //��ϢID
    uint8_t  valid;                  //������Ϣ�Ƿ���Ч
} MQTT_USER_MSG;

//������Ϣ�ṹ��
typedef struct
{
    int8_t topic[MSG_TOPIC_LEN];
    int8_t qos;
    int8_t retained;

    uint8_t msg[MSG_MAX_LEN];
    uint8_t msglen;
} mqtt_recv_msg_t, *p_mqtt_recv_msg_t, mqtt_send_msg_t, *p_mqtt_send_msg_t;


void mqtt_thread( void *pvParameters);

/************************************************************************
** ��������: my_mqtt_send_pingreq
** ��������: ����MQTT������
** ��ڲ���: ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
int32_t MQTT_PingReq(int32_t sock);

/************************************************************************
** ��������: MQTT_Connect
** ��������: ��¼������
** ��ڲ���: int32_t sock:����������
** ���ڲ���: Connect_OK:��½�ɹ� ����:��½ʧ��
** ��    ע:
************************************************************************/
uint8_t MQTT_Connect(void);

/************************************************************************
** ��������: MQTTSubscribe
** ��������: ������Ϣ
** ��ڲ���: int32_t sock���׽���
**           int8_t *topic������
**           enum QoS pos����Ϣ����
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
int32_t MQTTSubscribe(int32_t sock, char *topic, enum QoS pos);

/************************************************************************
** ��������: UserMsgCtl
** ��������: �û���Ϣ������
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void UserMsgCtl(MQTT_USER_MSG  *msg);

/************************************************************************
** ��������: GetNextPackID
** ��������: ������һ�����ݰ�ID
** ��ڲ���: ��
** ���ڲ���: uint16_t packetid:������ID
** ��    ע:
************************************************************************/
uint16_t GetNextPackID(void);

/************************************************************************
** ��������: mqtt_msg_publish
** ��������: �û�������Ϣ
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע:
************************************************************************/
//int32_t MQTTMsgPublish(int32_t sock, char *topic, int8_t qos, int8_t retained,uint8_t* msg,uint32_t msg_len);
int32_t MQTTMsgPublish(int32_t sock, char *topic, int8_t qos, uint8_t* msg);
/************************************************************************
** ��������: ReadPacketTimeout
** ��������: ������ȡMQTT����
** ��ڲ���: int32_t sock:����������
**           uint8_t *buf:���ݻ�����
**           int32_t buflen:��������С
**           uint32_t timeout:��ʱʱ��--0-��ʾֱ�Ӳ�ѯ��û��������������
** ���ڲ���: -1������,����--������
** ��    ע:
************************************************************************/
int32_t ReadPacketTimeout(int32_t sock, uint8_t *buf, int32_t buflen, uint32_t timeout);

/************************************************************************
** ��������: mqtt_pktype_ctl
** ��������: ���ݰ����ͽ��д���
** ��ڲ���: uint8_t packtype:������
** ���ڲ���: ��
** ��    ע:
************************************************************************/
void mqtt_pktype_ctl(uint8_t packtype, uint8_t *buf, uint32_t buflen);

/************************************************************************
** ��������: WaitForPacket
** ��������: �ȴ��ض������ݰ�
** ��ڲ���: int32_t sock:����������
**           uint8_t packettype:������
**           uint8_t times:�ȴ�����
** ���ڲ���: >=0:�ȵ����ض��İ� <0:û�еȵ��ض��İ�
** ��    ע:
************************************************************************/
int32_t WaitForPacket(int32_t sock, uint8_t packettype, uint8_t times);


void
mqtt_thread_init(void);

#endif



