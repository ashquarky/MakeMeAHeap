# MakeMeAHeap
A Wii U library to make you a MEM2 heap in a dirty userspace environment. Heaps around 160MB are common.

## Usage
Simply include `heapcreate.c` and `heapcreate.h` in your project, then call the functions inside as appropriate.

Check out the `src` folder for a quick demo. The Makefile provided should build it for you.

So far, I have only been able to get this to work as a 5.5.x ELF. I have no clue why.