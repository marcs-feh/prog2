#ifndef _arena_h_include_
#define _arena_h_include_

#include "base.h"
#include "memory.h"
#include "virtual_memory.h"

typedef struct Arena Arena;

typedef enum ArenaKind ArenaKind;

enum ArenaKind {
	ArenaKind_Buffer = 0,  // Uses single fixed length buffer, it's the most basic type of arena.
	ArenaKind_Virtual = 1, // Uses a single buffer with a big reserved address space, committing pages as necessary
};

#define ARENA_VIRTUAL_BLOCK_SIZE (16 * KiB)

struct Arena {
	MemoryBlock data;
	Size offset;
	U8 kind;
	Uintptr last_allocation;
};

// Helper macro
#define arena_push(A, Type, Count) ((Type *)arena_alloc((A), sizeof(Type) * (Count), alignof(Type)))

// Initialize a memory arena from a buffer
bool arena_init_buffer(Arena* a, U8* data, Size len);

// Initialize a memory arena with a reserved virtual address space
bool arena_init_virtual(Arena* a, Size reserve);

// Deinit the arena
void arena_destroy(Arena *a);

// Resize arena allocation in-place, gives back same pointer on success, null on failure
void* arena_resize(Arena* a, void* ptr, Size new_size);

// Try to resize allocation in-place, otherwhise re-allocates
void* arena_realloc(Arena* a, void* ptr, Size old_size, Size new_size, Size align);

// Reset arena, marking all its owned pointers as freed
void arena_free_all(Arena* a);

// Allocate `size` bytes aligned to `align`, return null on failure
void *arena_alloc(Arena* a, Size size, Size align);

#endif /* Include guard */
