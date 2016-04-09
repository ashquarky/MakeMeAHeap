/*
Make Me A Heap v1.0 - A Wii U library to make large heaps in a dirty userspace environment.

Distributed under DBaJ 1.2; please see https://github.com/QuarkTheAwesome/MakeMeAHeap/raw/master/LICENSE.md for more details.

This file is part of the demo to show the library's usage. It should be ran with yellows8's browserhax exploit.
*/

#include "init.h"
#include "loader.h"

/*
	Include the library itself.
	If you have it in a seperate location (like I do here) then you'll need to modify the Makefile to match.
	Otherwise, just copy it into the same location as your code ;3
*/
#include "../heapcreate.h"

void demoMain() {
	char print[255]; //Useful to have, not really needed
	
	printstr(0, "Make Me A Heap Demo - by Ash (@QuarkTheAwesome)");
	printstr(1, "INFO: Cleaning old heaps... (takes a while)");
	
	//MemoryBorders is a struct defined by the library, containing the low and high ends of the clean memory.
	//cleanUpMEM2 searches for old Expanded and Frame heaps, deletes them, and returns a MemoryBorders struct to match.
	MemoryBorders borders = cleanUpMEM2();
	
	__os_snprintf(print, 255, "OK: Cleaned old heaps between 0x%X and 0x%X", borders.lowEnd, borders.highEnd);
	printstr(2, print);
	printstr(3, "INFO: Creating new heap...");
	
	//createExpHeap is a convenience function. It's probably faster just to create the heap yourself.
	void* largeHeap = createExpHeap(borders);
	
	/* Alternativley to the above, you can just do something like
	
	void* largeHeap = cleanUpMEM2AndCreateExpHeap(); //Convenience function
	
	Which is a lot neater. */
	
	unsigned int (*MEMGetTotalFreeSizeForExpHeap)(void* heap);
	OSDynLoad_FindExport(coreinit_handle, 0, "MEMGetTotalFreeSizeForExpHeap", &MEMGetTotalFreeSizeForExpHeap);
	
	unsigned int heapSize = MEMGetTotalFreeSizeForExpHeap(largeHeap);
	
	__os_snprintf(print, 255, "OK: Created a %.2fMB (0x%X) heap at 0x%X", ((float)heapSize)/1024/1024, heapSize, largeHeap);
	printstr(4, print);
	
	printstr(5, "All OK! Press A to quit.");
	waitUntilVPAD();
}

/*
	Standard Wii U initialisation stuff. Nothing unexpected here!
*/
void _main() {
	void(*OSScreenInit)();
	unsigned int(*OSScreenGetBufferSizeEx)(unsigned int bufferNum);
	unsigned int(*OSScreenSetBufferEx)(unsigned int bufferNum, void * addr);
	
	void*(*OSAllocFromSystem)(uint32_t size, int align);
	void(*OSFreeToSystem)(void *ptr);
	
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenInit", &OSScreenInit);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenGetBufferSizeEx", &OSScreenGetBufferSizeEx);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenSetBufferEx", &OSScreenSetBufferEx);

	OSDynLoad_FindExport(coreinit_handle, 0, "OSAllocFromSystem", &OSAllocFromSystem);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSFreeToSystem", &OSFreeToSystem);
	
	OSScreenInit();
	
	int buf0_size = OSScreenGetBufferSizeEx(0);
	
	OSScreenSetBufferEx(0, (void *)0xF4000000);
	OSScreenSetBufferEx(1, (void *)0xF4000000 + buf0_size);
	
	clearScreen(); //I suppose this may be unexpected
	
	demoMain();
}

/*
	More standard stuff.
*/
void flipBuffers()
{
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);

	void(*DCFlushRange)(void *buffer, uint32_t length);
	unsigned int(*OSScreenFlipBuffersEx)(unsigned int bufferNum);
	OSDynLoad_FindExport(coreinit_handle, 0, "DCFlushRange", &DCFlushRange);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenFlipBuffersEx", &OSScreenFlipBuffersEx);
	unsigned int(*OSScreenGetBufferSizeEx)(unsigned int bufferNum);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenGetBufferSizeEx", &OSScreenGetBufferSizeEx);
	//Grab the buffer size for each screen (TV and gamepad)
	int buf0_size = OSScreenGetBufferSizeEx(0);
	int buf1_size = OSScreenGetBufferSizeEx(1);
	//Flush the cache
	DCFlushRange((void *)0xF4000000 + buf0_size, buf1_size);
	DCFlushRange((void *)0xF4000000, buf0_size);
	//Flip the buffer
	OSScreenFlipBuffersEx(0);
	OSScreenFlipBuffersEx(1);
}

void drawString(int x, int line, char * string)
{
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	unsigned int(*OSScreenPutFontEx)(unsigned int bufferNum, unsigned int posX, unsigned int line, void * buffer);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenPutFontEx", &OSScreenPutFontEx);
	OSScreenPutFontEx(0, x, line, string);
	OSScreenPutFontEx(1, x, line, string);
}

inline void printstr(int line, char * string) {
	drawString(0, line, string);
	flipBuffers();
	drawString(0, line, string);
	flipBuffers();
}

void fillScreen(char r,char g,char b,char a)
{
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	unsigned int(*OSScreenClearBufferEx)(unsigned int bufferNum, unsigned int temp);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenClearBufferEx", &OSScreenClearBufferEx);
	uint32_t num = (r << 24) | (g << 16) | (b << 8) | a;
	OSScreenClearBufferEx(0, num);
	OSScreenClearBufferEx(1, num);
}

inline void clearScreen() {
	fillScreen(0,0,0,0);
	flipBuffers();
	fillScreen(0,0,0,0);
	flipBuffers();
}

/*
	Quick, dirty, ugly VPAD reader. Waits until A is pressed. Please NEVER read or mention this function again.
*/
void waitUntilVPAD() {
	typedef struct
	{
		float x,y;
	} Vec2D;

	typedef struct
	{
		uint16_t x, y;               /* Touch coordinates */
		uint16_t touched;            /* 1 = Touched, 0 = Not touched */
		uint16_t validity;           /* 0 = All valid, 1 = X invalid, 2 = Y invalid, 3 = Both invalid? */
	} VPADTPData;
 
	typedef struct
	{
		uint32_t btn_hold;           /* Held buttons */
		uint32_t btn_trigger;        /* Buttons that are pressed at that instant */
		uint32_t btn_release;        /* Released buttons */
		Vec2D lstick, rstick;        /* Each contains 4-byte X and Y components */
		char unknown1c[0x52 - 0x1c]; /* Contains accelerometer and gyroscope data somewhere */
		VPADTPData tpdata;           /* Normal touchscreen data */
		VPADTPData tpdata1;          /* Modified touchscreen data 1 */
		VPADTPData tpdata2;          /* Modified touchscreen data 2 */
		char unknown6a[0xa0 - 0x6a];
		uint8_t volume;
		uint8_t battery;             /* 0 to 6 */
		uint8_t unk_volume;          /* One less than volume */
		char unknowna4[0xac - 0xa4];
	} VPADData;
	unsigned int vpad_handle;
	OSDynLoad_Acquire("vpad.rpl", &vpad_handle);
	int(*VPADRead)(int controller, VPADData *buffer, unsigned int num, int* error);
	OSDynLoad_FindExport(vpad_handle, 0, "VPADRead", &VPADRead);
	
	VPADData vpad;
	int error;
	VPADRead(0, &vpad, 1, &error);
	while (1) {
		VPADRead(0, &vpad, 1, &error);
		if (vpad.btn_hold & 0x8000) {
			break;
		}
	}
}