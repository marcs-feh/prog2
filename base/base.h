#ifndef _base_h_include_
#define _base_h_include_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <limits.h>

// DO **NOT** use <stdnoreturn.h> it is broken on windows.

#define nil NULL

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef ptrdiff_t Size; // Signed size
typedef uintptr_t Uintptr;

typedef U32 Rune;

typedef float  F32;
typedef double F64;

typedef _Atomic(uint8_t)  AtomicU8;
typedef _Atomic(uint16_t) AtomicU16;
typedef _Atomic(uint32_t) AtomicU32;
typedef _Atomic(uint64_t) AtomicU64;

typedef _Atomic(int8_t)  AtomicI8;
typedef _Atomic(int16_t) AtomicI16;
typedef _Atomic(int32_t) AtomicI32;
typedef _Atomic(int64_t) AtomicI64;

typedef _Atomic(Size)    AtomicSize;
typedef _Atomic(Uintptr) AtomicUintptr;

// This is to avoid conflict with stdlib's "abs()"
#define abs_val(X) (( (X) < 0ll) ? -(X) : (X))

#define min(A, B)  (((A) < (B)) ? (A) : (B))
#define max(A, B)  (((A) > (B)) ? (A) : (B))

#define clamp(Lo, X, Hi) min(max(Lo, X), Hi)

#define container_of(Ptr, Type, Member) \
	((Type *)(((void *)(Ptr)) - offsetof(Type, Member)))

#ifndef __cplusplus
#undef bool
typedef _Bool bool;
#define static_assert(Pred, Msg) _Static_assert(Pred, Msg)
#endif

#define hint_likely(X) __builtin_expect(!!(X), 1)
#define hint_unlikely(X) __builtin_expect(!!(X), 0)

static_assert(sizeof(F32) == 4 && sizeof(F64) == 8, "Bad float size");
static_assert(sizeof(void(*)(void)) == sizeof(void*), "Function pointers and data pointers must be of the same width");
static_assert(sizeof(void(*)(void)) == sizeof(Uintptr), "Mismatched pointer types");
static_assert(sizeof(Size) == sizeof(size_t), "Mismatched size");
static_assert(CHAR_BIT == 8, "Invalid char size");

#ifndef NO_STDIO
extern int printf(const char*, ...);
#endif

#if __STDC_VERSION__ < 202311L
	#define typeof(X) __typeof__(X)
#endif

static inline _Noreturn
void panic(char const * msg) {
	#ifndef NO_STDIO
	if(msg)
		printf("Panic: %s\n", msg);
	#else
	(void)msg;
	#endif
	__builtin_trap();
}

static inline
void ensure(bool pred, char const * msg){
	if(!(pred)){
		#ifndef NO_STDIO
			printf("Assertion failed: %s\n", msg);
		#else
			(void)msg;
		#endif
		panic(NULL);
	}
}

typedef struct String String;

#define str_literal(cslit) (String){ .v = (uint8_t const*)(cslit), .len = (sizeof(cslit) - 1) }

struct String {
	U8 const * v;
	Size len;
};

#endif /* Include guard */
