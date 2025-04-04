typedef enum {
	META_TYPE_u8,
	META_TYPE_u16,
	META_TYPE_u32,
	META_TYPE_u64,
	META_TYPE_s8,
	META_TYPE_s16,
	META_TYPE_s32,
	META_TYPE_s64,
	META_TYPE_f32,
	META_TYPE_f64,
	META_TYPE_bool,
	META_TYPE_u8_ptr,
	META_TYPE_uint64_t,
	META_TYPE_Arena_ptr
} MetaType;

typedef struct {
	MetaType type;
	String member_name;
	u8 offset;
} StructMembers;

StructMembers Struct_Arena[] = {
	{ META_TYPE_u8_ptr, strlit("buffer"), offsetof(Arena, buffer) },
	{ META_TYPE_uint64_t, strlit("buffer_size"), offsetof(Arena, buffer_size) },
	{ META_TYPE_uint64_t, strlit("current_offset"), offsetof(Arena, current_offset) }
};

StructMembers Struct_Temp[] = {
	{ META_TYPE_Arena_ptr, strlit("arena"), offsetof(Temp, arena) },
	{ META_TYPE_uint64_t, strlit("original_offset"), offsetof(Temp, original_offset) }
};

StructMembers Struct_Context[] = {
	{ META_TYPE_Arena_ptr, strlit("global_arena"), offsetof(Context, global_arena) },
	{ META_TYPE_Arena_ptr, strlit("scratch_pool"), offsetof(Context, scratch_pool) }
};
