#include "base/base.h"
#include "base/arena.h"

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
	U8 kind;
};

struct TokenArray {
	Token* v;
	Size   len;
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

Token lexer_next(Lexer* lex){
	U8 c = lexer_advance(lex);
	Token tk = {0};

	switch(c){
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

void lexer_push_error(Lexer* lex, String msg, Arena* arena){
	Error* e = arena_push(arena, Error, 1);

	if(e == NULL) { return; }
	e->description = msg;
}

int main(){}

