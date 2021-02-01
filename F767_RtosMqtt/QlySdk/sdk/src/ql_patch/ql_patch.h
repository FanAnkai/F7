#ifndef __QL_PATCH__
#define __QL_PATCH__

#include "ql_include.h"



#define PATCH_CHECK_TYPE_TIMER          0
#define PATCH_CHECK_TYPE_PASSIVE        1

#define PATCH_CHECK_TYPE_NEED_PASSIVE   2
#define PATCH_CHECK_TYPE_NEED_COMPELS   3
#define PATCH_CHECK_TYPE_COMPELS        9

#define QL_OP_PATCH_PASSIVE             0x0a10
#define QL_OP_PATCHS_CHECK_LIST         0x0a11
#define QL_OP_PATCH_CHUNK               0x0a13
#define QL_OP_PATCH_END                 0x0a14

#define PATCH_CHECK_DELAY               21600

#define QL_PATCHING_MAXTIME             120

typedef unsigned int ( *get_seq_t ) ( void );
typedef unsigned int ( *get_time_t ) ( void );
typedef ota_res_arg patch_res_arg;

typedef enum PATCH_STATE {
    PATCH_STATE_INIT,
    PATCH_STATE_ING,
    PATCH_STATE_DOWNLOADED,
}PATCH_STATE_T;

typedef struct _patch_download_ctx {
    type_u8          patching_index;
    type_u32         cur_len;
    type_u32         exptect_offset;
    type_u16         cur_crc;
} patch_download_ctx;

typedef struct ql_patch_s{
    patchs_list_t*           patchs_list;
    type_u32*                patchs_crc;
    cJSON *                  patch_json;
    type_u8                  patchs_count;

    type_u8                  patch_status;
    type_u8                  patch_upgrade_type;
    type_u32                 patch_chunk_size;

    type_u32                 patch_stat_chg_tm;
    type_u32                 patch_check_tm;

    get_seq_t                patch_get_seq;
    get_time_t               patch_get_time;
}ql_patch_t;




/* for patch */
#define WEAK_S  __attribute__((weak))
WEAK_S void ql_patch_init( type_u32 chunk_size, get_seq_t get_seq, get_time_t get_time );
WEAK_S void ql_patch_handle(type_u16 op_code, const char *in, int inlen, type_u32 pkt_seq);
WEAK_S void ql_patch_main( void );
WEAK_S type_s32 ql_patch_statu_get();
WEAK_S void ql_patch_check_tm_set(type_u32 check_tm);
/* end patch */

#endif

