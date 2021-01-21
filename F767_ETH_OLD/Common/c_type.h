/*******************************************************************************
* Filename:		cType.h
* Path:			D:\PROJECT\Lifter\SoftWare\100\Lifter\Sys
* Author:		ZHENG
* Data:			2016-1-29
* Last Update:	2016-1-29
* Lib Version:	ST3.5.0
* Version:		V1.0
* Brief:		常用的自定义类型
* History:		-Version:
*				-Update:
*				-Brief:
*******************************************************************************/

#ifndef __CTYPE_H
#define __CTYPE_H

/* Exported Includes ---------------------------------------------------------*/
//#include "config.h"
/* Exported Define -----------------------------------------------------------*/
#ifndef NULL
#define NULL		0
#endif // !NULL

#ifndef null
#define null		0
#endif // !null

#ifndef true
#define true		1
#define false		0
#endif // !true

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif // !TRUE



/* Exported Typedef ----------------------------------------------------------*/
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef float				f32;

typedef const unsigned char uc8;

typedef unsigned char		U8;
typedef unsigned char		BOOL;
typedef unsigned char		bool;
typedef unsigned short		U16;
typedef unsigned int		U32;
typedef float				F32;

typedef char				s8;
typedef short				s16;
typedef int					s32;

typedef char				S8;
typedef short				S16;
typedef int					S32;

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;

typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
//typedef unsigned int		UINT32;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;

typedef unsigned char		UINT8_t;
typedef unsigned short		UINT16_t;
typedef unsigned int		UINT32_t;

typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned int		DWORD;

typedef unsigned char		uchar;
typedef unsigned char		UCHAR;


typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned int		dword;

typedef unsigned char *		pu8;
typedef unsigned short *	pu16;
typedef unsigned int *		pu32;

typedef unsigned char *		pU8;
typedef unsigned short *	pU16;
typedef unsigned int *		pU32;

typedef unsigned char *		puint8;
typedef unsigned short *	puint16;
typedef unsigned int *		puint32;

typedef unsigned char *		pUINT8;
typedef unsigned short *	pUINT16;
typedef unsigned int *		pUINT32;

typedef unsigned char *		puint8_t;
typedef unsigned short *	puint16_t;
typedef unsigned int *		puint32_t;

typedef unsigned char *		pUINT8_t;
typedef unsigned short *	pUINT16_t;
typedef unsigned int *		pUINT32_t;

typedef unsigned char *		pBYTE;
typedef unsigned short *	pWORD;
typedef unsigned int *		pDWORD;

typedef unsigned char *		pbyte;
typedef unsigned short *	pword;
typedef unsigned int *		pdword;

typedef unsigned char *		Pu8;
typedef unsigned short *	Pu16;
typedef unsigned int *		Pu32;

typedef unsigned char *		PU8;
typedef unsigned short *	PU16;
typedef unsigned int *		PU32;

typedef unsigned char *		Puint8;
typedef unsigned short *	Puint16;
typedef unsigned int *		Puint32;

typedef unsigned char *		PUINT8;
typedef unsigned short *	PUINT16;
//typedef unsigned int *		PUINT32;

typedef unsigned char *		Puint8_t;
typedef unsigned short *	Puint16_t;
typedef unsigned int *		Puint32_t;

typedef unsigned char *		PUINT8_t;
typedef unsigned short *	PUINT16_t;
typedef unsigned int *		PUINT32_t;

typedef unsigned char *		PBYTE;
typedef unsigned short *	PWORD;
typedef unsigned int *		PDWORD;

typedef unsigned char *		Pbyte;
typedef unsigned short *	Pword;
typedef unsigned int *		Pdword;

typedef union
{
    unsigned short SHORT;
    struct
    {
        unsigned char LOW;
        unsigned char HIGH;
    } BYTE;
}
u16_LittleEndian_t;
typedef u16_LittleEndian_t *pu16_LittleEndian_t;

typedef union
{
    unsigned short SHORT;
    struct
    {
        unsigned char HIGH;
        unsigned char LOW;
    } BYTE;
}
u16_BigEndian_t;
typedef u16_BigEndian_t *pu16_BigEndian_t;

/* Exported Macro ------------------------------------------------------------*/
/* Exported Variables --------------------------------------------------------*/
/* Exported Function prototype -----------------------------------------------*/
/* Exported Function ---------------------------------------------------------*/


#endif /* __CTYPE_H */

/*******************************  END OF FILE  ********************************/
