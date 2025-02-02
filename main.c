#include "base/base.h"
#include "base/strings.h"
#include "base/arena.h"
#include <stdio.h>

typedef struct Lexer Lexer;
typedef struct Token Token;
typedef struct TokenArray TokenArray;
typedef struct Error Error;

struct Error {
	String description;
	Size   offset;
	Error* next;
};

struct Lexer {
	String source;
	Size   current;
	Size   previous;
	Error* error_head;
	Arena* arena;
};

typedef enum {
	TK_Unknown = 0,

	TK_Identifier, TK_String, TK_Number,
	TK_Nil, TK_True, TK_False,

	TK_CurlyOpen, TK_CurlyClose,
	TK_SquareOpen, TK_SquareClose,

	TK_Comma, TK_Colon,

	TK_Comment,

	TK_EndOfFile = -1,
	TK_Error = -2,
} TokenKind;

struct Token {
	String lexeme;
	U64    offset;
	I32    kind;
};

struct TokenArray {
	Token* v;
	Size   len;
	Size   cap;
	Arena* arena;
};

U8 lexer_advance(Lexer* lex){
	if(lex->current >= lex->source.len){
		return 0;
	}
	lex->current += 1;
	return lex->source.v[lex->current - 1];
}

U8 lexer_peek(Lexer* lex){
	if(lex->current >= lex->source.len){
		return 0;
	}
	return lex->source.v[lex->current];
}

static inline
bool is_number(U8 c){
	return (c >= '0') && (c <= '9');
}

static inline
bool is_whitespace(U8 c){
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

Token lexer_next(Lexer* lex){
	U8 c = 0;
	Token tk = {0};

	// Skip whitespaces
	for(c = lexer_advance(lex); is_whitespace(c); c = lexer_advance(lex)){
	}

	switch(c){
		case   0: tk.kind = TK_EndOfFile; return tk;
		case '{': tk.kind = TK_CurlyOpen; return tk;
		case '}': tk.kind = TK_CurlyClose; return tk;
		case '[': tk.kind = TK_SquareOpen; return tk;
		case ']': tk.kind = TK_SquareClose; return tk;
		case ':': tk.kind = TK_Colon; return tk;
		case ',': tk.kind = TK_Comma; return tk;

	}

	if(c == '"'){
		panic("strings");
		return tk;
	}

	if(is_number(c)){
		panic("numbers");
		return tk;
	}

	{
		panic("Identifiers");
		return tk;
	}

	tk.kind = TK_Error;
	return tk;
}

void lexer_push_error(Lexer* lex, String msg, Size offset, Arena* arena){
	Error* e = arena_push(arena, Error, 1);

	if(e == NULL) { return; }
	e->description = str_clone(msg, arena);
	e->next = lex->error_head;
	e->offset = offset;
}

String EXAMPLE_SRC = str_literal(
	"{ : , [] }"
);

#define DYN_ARRAY_MIN_CAP 16

#define dyn_array_resize(ArrPtr, NewCap) do {                                                           \
	Size _da_tmp_new_cap_ = max(DYN_ARRAY_MIN_CAP, NewCap);                                             \
	void* _da_tmp_new_data_ = arena_realloc((ArrPtr)->arena,                                            \
		(ArrPtr)->v,                                                                                    \
		(ArrPtr)->cap * sizeof(typeof(*(ArrPtr)->v)),                                                   \
		_da_tmp_new_cap_ * sizeof(typeof(*(ArrPtr)->v)),                                                \
		alignof(typeof(*(ArrPtr)->v)));                                                                 \
															                                            \
	if(hint_likely(_da_tmp_new_data_ != NULL)){                                                         \
		Size _da_tmp_copy_count_ = sizeof(typeof(*(ArrPtr)->v)) * min((ArrPtr)->len, _da_tmp_new_cap_); \
		mem_copy(_da_tmp_new_data_, (ArrPtr)->v, _da_tmp_copy_count_);                                  \
		(ArrPtr)->v = _da_tmp_new_data_;                                                                \
		(ArrPtr)->cap = _da_tmp_new_cap_;                                                               \
	}                                                                                                   \
} while(0)

#define dyn_array_push(ArrPtr, Elem) do {                  \
	if(((ArrPtr)->len >= (ArrPtr)->cap)){                  \
		dyn_array_resize((ArrPtr), (ArrPtr)->cap * 2);     \
	}                                                      \
	(ArrPtr)->v[(ArrPtr)->len] = Elem;                     \
	(ArrPtr)->len += 1;                                    \
} while(0)

#define dyn_array_pop(ArrPtr) do {      \
	if(hint_likely((ArrPtr)->len > 0)){ \
		(ArrPtr)->len -= 1;             \
	}                                   \
} while(0)

typedef struct {
	F32* v;
	Size len;
	Size cap;
	Arena* arena;
} F32Array;

static inline
void print_array(F32Array arr){
	printf("cap:%ld len:%ld [", arr.cap, arr.len);
	for(Size i = 0; i < arr.len; i++){
		if(i > 0) putc(' ', stdout);
		printf("%.2f", arr.v[i]);
	}
	printf("]\n");
}

int main(){
	Arena main_arena = {0};
	Arena temp_arena = {0};

	if(!arena_init_virtual(&main_arena, 512 * MiB)){
		panic("Failed to reserve virtual memory");
	}
	if(!arena_init_virtual(&temp_arena, 16 * MiB)){
		panic("Failed to reserve virtual memory");
	}

	// const Size initial_token_buf = 1024;
	// TokenArray tokens = {
	// 	.v = arena_push(&main_arena, Token, initial_token_buf),
	// 	.len = 0,
	// 	.cap = initial_token_buf,
	// 	.arena = &main_arena,
	// };
	//
	// Lexer lex = {
	// 	.source = EXAMPLE_SRC,
	// 	.current = 0,
	// 	.previous = 0,
	// 	.error_head = NULL,
	// 	.arena = &main_arena,
	// };
	//
	// while(1){
	// 	Token tk = lexer_next(&lex);
	// 	if(tk.kind == TK_EndOfFile){ break; }
	//
	// 	dyn_array_push(&tokens, tk);
	// }

	F32Array arr = {
		.v = NULL,
		.len = 0,
		.cap = 0,
		.arena = &main_arena,
	};

	for(int i = 0; i < 32; i++)
		dyn_array_push(&arr, 1.0 / (i+1));
	print_array(arr);
}

