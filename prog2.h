#ifndef _prog2_h_include_
#define _prog2_h_include_

#include <stdint.h>
#include <stddef.h>

/* Essential Types */
#define uintptr uintptr_t
#define isize   ptrdiff_t
#define byte    uint8_t

typedef struct Arena Arena;

struct Arena {
	isize offset;
	isize capacity;
	uintptr last_allocation;
	byte* data;
	Arena* next;
};

// Resize arena allocation in-place, gives back same pointer on success, null on failure
void* arena_resize(Arena* ar, void* ptr, isize new_size);

// Reset arena, marking all its owned pointers as freed
void arena_free_all(Arena* ar);

// Allocate `size` bytes aligned to `align`, return null on failure
void* arena_alloc(Arena* ar, isize size, isize align);

#undef uintptr
#undef isize
#undef byte

#endif /* Include guard */
