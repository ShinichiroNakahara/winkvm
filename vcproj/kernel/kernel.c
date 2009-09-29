/*
 * kernel.c
 * This file has the main routines of WinKVM
 *
 * Copyright (C) Kazushi Takahashi <kazushi@rvm.jp>, 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdarg.h>

#include "kernel.h"

#include <linux/winkvm.h>

#define MAX_PRINT_BUFFER 1024

#define  PAU64(x)      \
	((__u64)((x)->u.HighPart) << 32) | ((x)->u.LowPart)

int _cdecl printk(const char *s, ...)
{
	va_list argList;
	char szMsg[MAX_PRINT_BUFFER];

	va_start(argList, s);
	vsprintf(szMsg, s, argList);

	va_end(argList);
	DbgPrint(szMsg);

	return 1;
}

hpa_t get_phys(PHYSICAL_ADDRESS *addr)
{
	return PAU64(addr);
}

void* _cdecl __va(unsigned long addr)
{	
	return (void*)addr;
}

u64 _cdecl __pa(hva_t virt)
{
	PHYSICAL_ADDRESS paddr;
	paddr = MmGetPhysicalAddress((PVOID)virt);
	return PAU64(&paddr);
}

int _cdecl pfn_valid(unsigned long pfn)
{
	SAFE_ASSERT(0);
	return 1;
}

int _cdecl cpu_to_node(int cpu)
{
	return cpu;
}

int atomic_dec_and_test(int *v)
{
	unsigned char c;

	FUNCTION_ENTER();

	__asm {
		lock dec v
		sete c
	};

	FUNCTION_EXIT();

	return c != 0;
}

int atomic_inc_and_test(int *v)
{
	unsigned char c;
	FUNCTION_ENTER();

	__asm {
		lock inc v
		sete c
	};
	
	FUNCTION_EXIT();

	return c != 0;
}

ULONG KeQueryActiveProcessorCountCompatible(OUT PKAFFINITY ActiveProcessors)
{	
#if (NTDDI_VERSION >= NTDDI_VISTA)
	/* return KeQueryActiveProcessorCount(ActiveProcessors); */
	return KeNumberProcessors;
#else
	ULONG NumberOfProcessors = 0;
	KAFFINITY Affinity = KeQueryActiveProcessors();

	if (ActiveProcessors) {
		*ActiveProcessors = Affinity;
	}

	for (; Affinity; Affinity >>= 1){
		if (Affinity & 1) {
			NumberOfProcessors++;
		}
	}
	return NumberOfProcessors;
#endif
}

void dump_hex(unsigned char *buffer, int len, int row)
{
	int i, j;
	for (i = 0, j = 1 ; i < len ; i++, j++) {		
		printk("0x%02x ", buffer[i]);
		if (j == row) {
			printk("\n");
			j = 1;
		}		
	}
}