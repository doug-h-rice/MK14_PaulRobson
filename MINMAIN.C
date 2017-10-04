/*
	An ultra-minimalist emulator 'shell' - no features other than
	emulation.

	Link with CPU.C MEMORY.C and DRV_xxx.C to build it.

*/

#include "scmp.h"

void main(int argc,char *argv[])
{
MinimalistEmulator(argc == 2 ? argv[1] : "");
}