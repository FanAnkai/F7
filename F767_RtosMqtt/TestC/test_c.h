
#ifndef TEST_C_H_
#define TEST_C_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef struct
{
	char a[9];
	double c;
	int b;
	
	
}test0_t;

typedef struct
{
	long long a;
	char b;
	
}test1_t;

typedef struct
{
	char a;
	long long b;
	char c;
	
}test2_t;


typedef struct
{
	char a:3;
	long long b:24;
	char c:4;
	
}test3_t;

typedef struct
{
	char a:3;
	long long b:24;
	char c:7;
	char d:7;
	char e:7;
	
}test4_t;

//有人说位域不能跨字节，下边证明可以
typedef struct
{
	char a:3;
	long long b:55;
	char c:6;
	
}test5_t;



typedef union
{
	char a[9];
	int b;
	double c;
	
}test_union;

#pragma pack(pop)



void test_c(void);


#endif

