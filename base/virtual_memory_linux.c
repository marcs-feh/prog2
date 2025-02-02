#if defined(TARGET_OS_LINUX)
#include "virtual_memory.h"
#include <sys/mman.h>
#include <unistd.h>

void virtual_init(){
	static bool initialized = false;
	if(!initialized){
		U32 page_size = getpagesize();
		if(page_size != VIRTUAL_PAGE_SIZE){
			panic("Virtual memory constraints were not satisfied.");
		}
		initialized = true;
	}
}

void* virtual_reserve(Size len){
	void* ptr = mmap(NULL, len, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return ptr;
}

void* virtual_commit(void* ptr, Size len){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	if(mprotect(ptr, len, PROT_READ | PROT_WRITE) < 0){
		return NULL;
	}
	return ptr;	
}

void virtual_decommit(void* ptr, Size len){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	mprotect(ptr, len, PROT_NONE);
	madvise(ptr, len, MADV_FREE);
}

void virtual_free(void* ptr, Size len){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	munmap(ptr, len);
}

static inline
U32 _virtual_protect_flags(U8 prot){
	U32 flag = 0;
	if(prot & MemoryProtection_Exec)
		flag |= PROT_EXEC;
	if(prot & MemoryProtection_Read)
		flag |= PROT_READ;
	if(prot & MemoryProtection_Write)
		flag |= PROT_WRITE;
	return flag;
}

bool virtual_protect(void* ptr, Size len, U8 prot){
	ensure(((Uintptr)ptr & (VIRTUAL_PAGE_SIZE - 1)) == 0, "Pointer is not aligned to page boundary");
	U32 flags = _virtual_protect_flags(prot);
	return mprotect(ptr, len, flags) >= 0;
}

#endif
