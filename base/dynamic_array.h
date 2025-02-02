#ifndef _dynamic_array_h_include_
#define _dynamic_array_h_include_
// "Dynamic Array" Helper for C
// The required fields are:
// {
//     T*   v
//     Size len;
//     Size cap;
//     Arena* arena;
// }

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

#endif /* Include guard */
