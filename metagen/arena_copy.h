#pragma once

typedef struct {
    unsigned char* buffer;
    uint64_t buffer_size;
    uint64_t current_offset;
} Arena;

typedef struct {
    Arena* arena;
    uint64_t original_offset;
} Temp;

typedef struct {
    Arena* global_arena;
    Arena* scratch_pool;
} Context;

static _Thread_local Context ctx;

void  arena_init(Arena* arena, uint64_t arena_size);
void  arena_delete(Arena* arena);
void* arena_alloc_aligned(Arena* arena, uintptr_t num_of_elem, uintptr_t elem_size, uintptr_t align_size);
void  arena_reset(Arena* arena);
void  arena_clean(Arena* arena);
bool  arena_increment_offset(Arena* arena, uint64_t increment_value);
Temp  temp_begin(Arena* arena);
void  temp_end(Temp temp);
Temp  scratch_get_free(Arena** arena_pool, int arena_pool_size, Arena** conflicting_arenas, int conflicting_num);
void  scratch_end(Temp temp_scratch);

#define push_array_align(arena, type, num, align) (type*)arena_alloc_aligned(arena, (num), sizeof(type), align)
#define push_array(arena, type, num) (type*)arena_alloc_aligned(arena, (num), sizeof(type), _Alignof(type))
#define push_string(arena, num) (char*)arena_alloc_aligned(arena, (num), sizeof(char), _Alignof(char))
#define push_struct(arena, type) (type*)arena_alloc_aligned(arena, 1, sizeof(type), _Alignof(type))
#define get_scratch(conflicting_arenas, num) scratch_get_free(ctx.scratch_pool, 2, conflicting_arenas, num)
