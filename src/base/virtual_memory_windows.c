#if defined(TARGET_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "base.h"
#include "virtual_memory.h"

void virtual_init(){
	static bool initialized = false;
	if(!initialized){
		initialized = true;
	}
}

void* virtual_reserve(Size len){
	len = align_forward_size(len, VIRTUAL_PAGE_SIZE);
	void* ptr = VirtualAlloc(NULL, len, MEM_RESERVE, PAGE_NOACCESS);
	return ptr;
}

bool virtual_protect(void* ptr, Size len, U8 prot){
	U32 flags = 0;

	switch(prot){
		case MemoryProtection_Read:
			flags = PAGE_READONLY;
		break;
		case (MemoryProtection_Read | MemoryProtection_Write):
			flags = PAGE_READWRITE;
		break;
		case MemoryProtection_Exec:
			flags = PAGE_EXECUTE;
		break;
		case (MemoryProtection_Exec | MemoryProtection_Read):
			flags = PAGE_EXECUTE_READ;
		break;
		case (MemoryProtection_Exec | MemoryProtection_Read | MemoryProtection_Write):
			flags = PAGE_EXECUTE_READWRITE;
		break;
	}
	if(flags == 0){
		return NULL;
	}

	VirtualAlloc(ptr, len, MEM_COMMIT, flags);
	return false;
}

void virtual_free(void* ptr, Size len){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	(void)len;
	VirtualFree(ptr, 0, MEM_RELEASE);
}

void virtual_decommit(void* ptr, Size len){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	VirtualFree(ptr, len, MEM_DECOMMIT);
}

void* virtual_commit(void* ptr, Size len){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	VirtualAlloc(ptr, len, MEM_COMMIT, PAGE_READWRITE);
}

#endif
