#include "base.h"
#include "memory.h"
#include "arena.h"
#include "strings.h"

#define UTF8_RANGE1 ((I32)0x7f)
#define UTF8_RANGE2 ((I32)0x7ff)
#define UTF8_RANGE3 ((I32)0xffff)
#define UTF8_RANGE4 ((I32)0x10ffff)

#define UTF16_SURROGATE1 ((I32)0xd800)
#define UTF16_SURROGATE2 ((I32)0xdfff)

#define UTF8_MASK2 (0x1f) /* 0001_1111 */
#define UTF8_MASK3 (0x0f) /* 0000_1111 */
#define UTF8_MASK4 (0x07) /* 0000_0111 */

#define UTF8_MASKX (0x3f) /* 0011_1111 */

#define UTF8_SIZE2 (0xc0) /* 110x_xxxx */
#define UTF8_SIZE3 (0xe0) /* 1110_xxxx */
#define UTF8_SIZE4 (0xf0) /* 1111_0xxx */

#define CONT (0x80)  /* 10xx_xxxx */

static inline
bool is_continuation_byte(Rune c){
	static const Rune CONTINUATION1 = 0x80;
	static const Rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

UTF8Encode utf8_encode(Rune c){
	UTF8Encode res = {0};

	if(is_continuation_byte(c) ||
	   (c >= UTF16_SURROGATE1 && c <= UTF16_SURROGATE2) ||
	   (c > UTF8_RANGE4))
	{
		return UTF8_ERROR_ENCODED;
	}

	if(c <= UTF8_RANGE1){
		res.len = 1;
		res.bytes[0] = c;
	}
	else if(c <= UTF8_RANGE2){
		res.len = 2;
		res.bytes[0] = UTF8_SIZE2 | ((c >> 6) & UTF8_MASK2);
		res.bytes[1] = CONT  | ((c >> 0) & UTF8_MASKX);
	}
	else if(c <= UTF8_RANGE3){
		res.len = 3;
		res.bytes[0] = UTF8_SIZE3 | ((c >> 12) & UTF8_MASK3);
		res.bytes[1] = CONT  | ((c >> 6) & UTF8_MASKX);
		res.bytes[2] = CONT  | ((c >> 0) & UTF8_MASKX);
	}
	else if(c <= UTF8_RANGE4){
		res.len = 4;
		res.bytes[0] = UTF8_SIZE4 | ((c >> 18) & UTF8_MASK4);
		res.bytes[1] = CONT  | ((c >> 12) & UTF8_MASKX);
		res.bytes[2] = CONT  | ((c >> 6)  & UTF8_MASKX);
		res.bytes[3] = CONT  | ((c >> 0)  & UTF8_MASKX);
	}
	return res;
}

static const UTF8Decode DECODE_ERROR = { .codepoint = UTF8_ERROR, .len = 1 };

UTF8Decode utf8_decode(U8 const* buf, Size len){
	UTF8Decode res = {0};
	if(buf == NULL || len <= 0){ return DECODE_ERROR; }

	U8 first = buf[0];

	if((first & CONT) == 0){
		res.len = 1;
		res.codepoint |= first;
	}
	else if ((first & ~UTF8_MASK2) == UTF8_SIZE2 && len >= 2){
		res.len = 2;
		res.codepoint |= (buf[0] & UTF8_MASK2) << 6;
		res.codepoint |= (buf[1] & UTF8_MASKX) << 0;
	}
	else if ((first & ~UTF8_MASK3) == UTF8_SIZE3 && len >= 3){
		res.len = 3;
		res.codepoint |= (buf[0] & UTF8_MASK3) << 12;
		res.codepoint |= (buf[1] & UTF8_MASKX) << 6;
		res.codepoint |= (buf[2] & UTF8_MASKX) << 0;
	}
	else if ((first & ~UTF8_MASK4) == UTF8_SIZE4 && len >= 4){
		res.len = 4;
		res.codepoint |= (buf[0] & UTF8_MASK4) << 18;
		res.codepoint |= (buf[1] & UTF8_MASKX) << 12;
		res.codepoint |= (buf[2] & UTF8_MASKX) << 6;
		res.codepoint |= (buf[3] & UTF8_MASKX) << 0;
	}
	else {
		return DECODE_ERROR;
	}

	// Validation step
	if(res.codepoint >= UTF16_SURROGATE1 && res.codepoint <= UTF16_SURROGATE2){
		return DECODE_ERROR;
	}
	if(res.len > 1 && !is_continuation_byte(buf[1])){
		return DECODE_ERROR;
	}
	if(res.len > 2 && !is_continuation_byte(buf[2])){
		return DECODE_ERROR;
	}
	if(res.len > 3 && !is_continuation_byte(buf[3])){
		return DECODE_ERROR;
	}

	return res;
}

// Steps iterator forward and puts Rune and Length advanced into pointers,
// returns false when finished.
bool utf8_iter_next(UTF8Iterator* iter, UTF8Decode* out){
	if(iter->current >= iter->len){ return 0; }

	*out = utf8_decode(&iter->data[iter->current], iter->len - iter->current);

	iter->current += out->len;

	return 1;
}

// Steps iterator backward and puts Rune and its length into pointers,
// returns false when finished.
bool utf8_iter_prev(UTF8Iterator* iter, UTF8Decode* out){
	if(iter->current <= 0){ return false; }

	iter->current -= 1;
	while(is_continuation_byte(iter->data[iter->current])){
		iter->current -= 1;
	}

	*out = utf8_decode(&iter->data[iter->current], iter->len - iter->current);
	return true;
}

#undef CONT

bool str_empty(String s){
	return s.len == 0 || s.v == NULL;
}

String str_from(char const * data){
	String s = {
		.v = (U8 const *)data,
		.len = cstring_len(data),
	};
	return s;
}

String str_from_bytes(U8 const* data, Size length){
	String s = {
		.v = (U8 const *)data,
		.len = length,
	};
	return s;
}

String str_from_range(char const * data, Size start, Size length){
	String s = {
		.v = (U8 const *)&data[start],
		.len = length,
	};
	return s;
}

Size str_codepoint_count(String s){
	UTF8Iterator it = str_iterator(s);

	Size count = 0;
	UTF8Decode dec = {0};
	while(utf8_iter_next(&it, &dec)){
		count += 1;
	}
	return count;
}

bool str_starts_with(String s, String prefix){
	if(prefix.len > s.len){ return false; }

	s = str_sub(s, 0, prefix.len);

	I32 res = mem_compare(prefix.v, s.v, prefix.len);
	return res == 0;
}

bool str_ends_with(String s, String suffix){
	if(suffix.len > s.len){ return false; }

	s = str_sub(s, s.len - suffix.len, suffix.len);

	I32 res = mem_compare(suffix.v, s.v, suffix.len);
	return res == 0;
}

Size str_codepoint_offset(String s, Size n){
	UTF8Iterator it = str_iterator(s);

	Size acc = 0;

	UTF8Decode dec = {0};
	do {
		if(acc == n){ break; }
		acc += 1;
	} while(utf8_iter_next(&it, &dec));

	return it.current;
}

// TODO: Handle length in codepoint count
String str_sub(String s, Size start, Size byte_count){
	if(start < 0 || byte_count < 0 || (start + byte_count) > s.len){ return (String){0}; }

	String sub = {
		.v = &s.v[start],
		.len = byte_count,
	};

	return sub;
}

bool str_eq(String a, String b){
	if(a.len != b.len){ return false; }

	for(Size i = 0; i < a.len; i += 1){
		if(a.v[i] != b.v[i]){ return false; }
	}

	return true;
}

String str_clone(String c, Arena* arena){
	String res = {0};
	U8* new_buf = arena_push(arena, U8, c.len + 1);
	if(new_buf != NULL){
		mem_copy_no_overlap(new_buf, c.v, c.len);
		new_buf[c.len] = 0;
		res.len = c.len;
		res.v = new_buf;
	}
	return res;
}

UTF8Iterator str_iterator(String s){
	return (UTF8Iterator){
		.current = 0,
		.len     = s.len,
		.data    = s.v,
	};
}

UTF8Iterator str_iterator_reversed(String s){
	return (UTF8Iterator){
		.current = s.len,
		.len     = s.len,
		.data    = s.v,
	};
}

#define MAX_CUTSET_LEN 64

String str_trim(String s, String cutset){
	String st = str_trim_leading(str_trim_trailing(s, cutset), cutset);
	return st;
}

String str_trim_leading(String s, String cutset){
	ensure(cutset.len <= MAX_CUTSET_LEN, "Cutset string exceeds MAX_CUTSET_LEN");

	Rune set[MAX_CUTSET_LEN] = {0};
	Size set_len = 0;
	Size cut_after = 0;

	/* Decode cutset */ {
		UTF8Decode dec = {0};
		UTF8Iterator iter = str_iterator(cutset);

		Size i = 0;
		while(utf8_iter_next(&iter, &dec) && i < MAX_CUTSET_LEN){
			set[i] = dec.codepoint;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		UTF8Decode dec = {0};
		UTF8Iterator iter = str_iterator(s);

		while(utf8_iter_next(&iter, &dec)){
			bool to_be_cut = false;
			for(Size i = 0; i < set_len; i += 1){
				if(set[i] == dec.codepoint){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_after += dec.len;
			}
			else {
				break; // Reached first Rune that isn't in cutset
			}

		}
	}

	return str_sub(s, cut_after, s.len - cut_after);
}

String str_trim_trailing(String s, String cutset){
	ensure(cutset.len <= MAX_CUTSET_LEN, "Cutset string exceeds MAX_CUTSET_LEN");

	Rune set[MAX_CUTSET_LEN] = {0};
	Size set_len = 0;
	Size cut_until = s.len;

	/* Decode cutset */ {
		UTF8Decode dec = {0};
		UTF8Iterator iter = str_iterator(cutset);

		Size i = 0;
		while(utf8_iter_next(&iter, &dec) && i < MAX_CUTSET_LEN){
			set[i] = dec.codepoint;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		UTF8Decode dec = {0};
		UTF8Iterator iter = str_iterator_reversed(s);

		while(utf8_iter_prev(&iter, &dec)){
			bool to_be_cut = false;
			for(Size i = 0; i < set_len; i += 1){
				if(set[i] == dec.codepoint){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_until -= dec.len;
			}
			else {
				break; // Reached first Rune that isn't in cutset
			}

		}
	}

	return str_sub(s, 0, cut_until);
}

#undef UTF8_RANGE1
#undef UTF8_RANGE2
#undef UTF8_RANGE3
#undef UTF8_RANGE4
#undef UTF16_SURROGATE1
#undef UTF16_SURROGATE2
#undef UTF8_MASK2
#undef UTF8_MASK3
#undef UTF8_MASK4
#undef UTF8_MASKX
#undef UTF8_SIZE2
#undef UTF8_SIZE3
#undef UTF8_SIZE4
#undef CONT

