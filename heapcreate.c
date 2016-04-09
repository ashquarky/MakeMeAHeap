/*
Make Me A Heap v1.0 - A Wii U library to make large heaps in a dirty userspace environment.

Distributed under DBaJ 1.2; please see https://github.com/QuarkTheAwesome/MakeMeAHeap/raw/master/LICENSE.md for more details.

This file is part of the library itself.
*/

#include "heapcreate.h"

/*
 * Header stuff we don't need public
 */
//Memory section to search in. Appears safe on 5.5.
#define MEMORY_SAFE_LOWEND 0x10000000
#define MEMORY_SAFE_HIGHEND 0x20000000

#define EXP_HEAP_HEADER 0x45585048
#define FRM_HEAP_HEADER 0x46524D48

#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, char *symbol, void *address))0x0102B828)
#define OSFatal ((void (*)(char* msg))0x01031618)
#define __os_snprintf ((int(*)(char* s, int n, const char * format, ... ))0x0102F160)

/*
 * Search through MEM2 and delete all Expanded and Frame heaps. Returns the range of clean memory (borders.lowEnd through to borders.highEnd)
 */
MemoryBorders cleanUpMEM2() {
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	void (*MEMDestroyExpHeap)(void* heap);
	void (*MEMDestroyFrmHeap)(void* heap);
	OSDynLoad_FindExport(coreinit_handle, 0, "MEMDestroyFrmHeap", &MEMDestroyFrmHeap);
	OSDynLoad_FindExport(coreinit_handle, 0, "MEMDestroyExpHeap", &MEMDestroyExpHeap);
	
	MemoryBorders borders;
	borders.lowEnd = (void*)0; //This will store the address of the BOTTOM of the good memory
	borders.highEnd = (void*)0; //This will store the address of the TOP of the good memory
	
	register void* highEndFAST = (void*)0; //Quick access stuff. Nice.
	register _Bool lowEndUnset = 1;
	register void* search;
	
	for (search = (void*)MEMORY_SAFE_LOWEND; search <= (void*)MEMORY_SAFE_HIGHEND; search += 4) {
		if (*(unsigned int*)search == EXP_HEAP_HEADER) { //If we found an Expanded Heap...
			MEMDestroyExpHeap(search); //Delete it
			highEndFAST = search; //Store its address as the top of good memory
			if (lowEndUnset) { //If this is the first heap we found...
				borders.lowEnd = search; //Store its address as the bottom of the good memory
				lowEndUnset = 0;
			}
		}
	}
	for (search = (void*)MEMORY_SAFE_LOWEND; search <= (void*)MEMORY_SAFE_HIGHEND; search += 4) {
		if (*(unsigned int*)search == FRM_HEAP_HEADER) {
			MEMDestroyFrmHeap(search);
			if (search > highEndFAST) {
				highEndFAST = search;
			}
			if (search < borders.lowEnd) {
				borders.lowEnd = search;
			}
		}
	}
	
	borders.highEnd = highEndFAST;
	
	//By the time we get here we should have the highEnd variable set to the address of the last heap we found
	
	return borders;
}

void* createExpHeap(MemoryBorders borders) {
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	void* (*MEMCreateExpHeapEx)(void* address, unsigned int size, unsigned short flags);
	OSDynLoad_FindExport(coreinit_handle, 0, "MEMCreateExpHeapEx", &MEMCreateExpHeapEx);
	
	return MEMCreateExpHeapEx(borders.lowEnd, (unsigned int)(borders.highEnd - borders.lowEnd), 0);
}

/*
 * Convenience function.
 */
void* cleanUpMEM2AndCreateExpHeap() {
	MemoryBorders borders;
	borders = cleanUpMEM2();
	return createExpHeap(borders);
}