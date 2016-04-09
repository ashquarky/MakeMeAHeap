#ifndef LOADER_H
#define LOADER_H

void _main();
void demoMain();

void flipBuffers();
void drawString(int x, int line, char* string);
inline void printstr(int line, char* text);
void fillScreen(char r, char g, char b, char a);
inline void clearScreen();
void waitUntilVPAD();

typedef unsigned long long uint64_t;
typedef long long int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

typedef uint32_t size_t;

typedef _Bool bool;
#define true 1
#define false 0
#define null 0

#define NULL (void*)0

#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, char *symbol, void *address))0x0102B828)
#define OSFatal ((void (*)(char* msg))0x01031618)
#define __os_snprintf ((int(*)(char* s, int n, const char * format, ... ))0x0102F160)

struct iovec
{
	void *buffer;
	int len;
	char unknown8[0xc-0x8];
};

typedef struct OSContext
{
	/* OSContext identifier */
    uint32_t tag1;
    uint32_t tag2;

    /* GPRs */
    uint32_t gpr[32];

	/* Special registers */
    uint32_t cr;
    uint32_t lr;
    uint32_t ctr;
    uint32_t xer;

    /* Initial PC and MSR */
    uint32_t srr0;
    uint32_t srr1;
} OSContext;

#endif /* LOADER_H */