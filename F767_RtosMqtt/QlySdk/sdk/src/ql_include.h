#ifndef QL_INCLUDE_H_
#define QL_INCLUDE_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//#define USE_SMPU

#ifdef USE_SMPU
#define SEC_DATA   __attribute__((section (".secdata")))
#define SEC_CODE   __attribute__((section (".sec_code"))) __attribute__ ((__used__))
#define SEC_CONST __attribute__((section (".sec_const")))
#else
#define SEC_DATA
#define SEC_CODE
#define SEC_CONST
#endif

#define QLY_SDK_LIB         (0)
#define QLY_SDK_BIN         (1)

typedef unsigned int        type_size_t;
typedef unsigned char       type_u8;
typedef signed char         type_s8;
typedef unsigned short      type_u16;
typedef signed short        type_s16;
typedef unsigned int        type_u32;
typedef signed int          type_s32;
typedef unsigned long long  type_u64;
typedef signed long long    type_s64;
typedef float               type_f32;
typedef double              type_f64;

#include "iot_cjson.h"
#include "iot_md5.h"
#include "iot_interface.h"
#include "iot_base64.h"
#include "iot_aes.h"

#include "ql_device_basic_impl.h"
#include "ql_device_thread_impl.h"
#include "ql_log.h"
#include "ql_util.h"
#include "ql_logic_context.h"
#include "ql_packet.h"

#endif /* QL_INCLUDE_H_ */


