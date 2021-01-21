#ifndef __NINEBOT_PROTOCOL_H__
#define __NINEBOT_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"


typedef void (*RX_FRAME_CB)();
typedef void (*TX_FRAME_FUN)(uint8_t *arg1, uint16_t arg2);

typedef enum
{
    PARSE_OK,
    PARSE_FAIL
} PARSE_RETURN;

typedef enum
{
    USE_SERIAL,
    USE_TCP
} SEND_RECV_TYPE;

typedef enum
{
    NOHEAD,
    GETHEAD,
    GETVERSION,
    GETLENHALF,
    GETLEN
} PARSE_DATA_STA;

typedef struct frame_struct
{
    uint8_t     Version;
    uint16_t    FrameDataLen;
    uint8_t     SourceID;
    uint8_t     TargetID;
    uint8_t     Cmd;
    uint16_t    DataIndex;
    uint8_t     *frame_data;
    TX_FRAME_FUN send_frame_fun;
} FRAME_STRUCT;

typedef struct parse_struct
{

    PARSE_DATA_STA  sta;
    uint16_t        frame_cnt;
    uint16_t        frame_data_len;
    uint16_t        frame_len;
    uint8_t         version;
    uint8_t         frame_buffer[FRAME_MAX];
    RX_FRAME_CB     rec_frame_cb;
    FRAME_STRUCT    frame_s;
    
} PARSE_STRUCT;

PARSE_RETURN creat_send_cmd(PARSE_STRUCT *parse_s, FRAME_STRUCT *frame_s);

PARSE_RETURN parse_struct_init(PARSE_STRUCT *parse_s);
void parse_data(PARSE_STRUCT *parse_s, uint8_t *receive_buffer, uint16_t receive_len);
void set_id(PARSE_STRUCT *parse_s, uint8_t local, uint8_t target);
void frame_set_send_fun(FRAME_STRUCT *frame_s, TX_FRAME_FUN send_frame_fun);
void parse_set_send_fun(PARSE_STRUCT *parse_s, TX_FRAME_FUN send_frame_fun);
void parse_set_rec_callback(PARSE_STRUCT *parse_s, RX_FRAME_CB rec_frame_cb);

#ifdef __cplusplus
}
#endif

#endif
