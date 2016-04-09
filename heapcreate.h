/*
Make Me A Heap v1.0 - A Wii U library to make large heaps in a dirty userspace environment.

Distributed under DBaJ 1.2; please see https://github.com/QuarkTheAwesome/MakeMeAHeap/raw/master/LICENSE.md for more details.

This file is part of the library itself.
*/

#ifndef HEAPCREATE_H
#define HEAPCREATE_H

typedef struct {
	void* lowEnd;
	void* highEnd;
} MemoryBorders;

MemoryBorders cleanUpMEM2();
void* cleanUpMEM2AndCreateExpHeap();

MemoryBorders cleanUpBucket();
void* cleanUpBucketAndCreateExpHeap();

void* createExpHeap(MemoryBorders borders);
#endif //HEAPCREATE_H