#include "util.h"
#include "arena.h"
#include "string.h"
#include "output.h"

void global_arena_setup(u64 global_arena_size, u64 scratch_arena_size) {
    ctx.global_arena = malloc(sizeof(Arena));
    arena_init(ctx.global_arena, global_arena_size);

    for (u32 i = 0; i < ArrayCount(ctx.scratch_pool); i++) {
        ctx.scratch_pool[i] = malloc(sizeof(Arena));
        arena_init(ctx.scratch_pool[i], scratch_arena_size);
    }
}

String load_file_into_string(Arena* arena, String file_path) {
    FILE* file = fopen(file_path.string, "rb");
    if (file == NULL) {
        return NULL_STRING;
    }

    fseek(file, 0, SEEK_END);
    u64 file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = push_string(arena, file_size);
    fread(buffer, sizeof(char), file_size, file);

    fclose(file);
    return (String) { buffer, file_size };
}

enum TokenType {
    TOKEN_UNKNOWN,
    TOKEN_IDENTIFIER,
    TOKEN_EOF,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_ASTERIX,
    TOKEN_U8,
    TOKEN_U16,
    TOKEN_U32,
    TOKEN_U64,
    TOKEN_S8,
    TOKEN_S16,
    TOKEN_S32,
    TOKEN_S64,
    TOKEN_F32,
    TOKEN_F64,
    TOKEN_BOOL,
    TOKEN_OPEN_SQUIGGLY_BRACE,
    TOKEN_CLOSE_SQUIGGLY_BRACE,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_OPEN_SQUARE_BRACE,
    TOKEN_CLOSE_SQUARE_BRACE,
    TOKEN_STRUCT_KEYWORD,
    TOKEN_TYPEDEF_KEYWORD,
    TOKEN_UNSIGNED_KEYWORD,
    TOKEN_STRUCT_NAME,
    TOKEN_START_COMMENT,
    TOKEN_NEWLINE,
    TOKEN_CARRIAGE_RETURN,
    TOKEN_FUNCTION_NAME // TODO(ali): Maybe support parsing functions?
};

char* TokenTypeStrings[]  = {
    "TOKEN_UNKNOWN",
    "TOKEN_IDENTIFIER",
    "TOKEN_EOF",
    "TOKEN_SEMICOLON",
    "TOKEN_COLON",
    "TOKEN_COMMA",
    "TOKEN_ASTERIX",
    "TOKEN_U8",
    "TOKEN_U16",
    "TOKEN_U32",
    "TOKEN_U64",
    "TOKEN_S8",
    "TOKEN_S16",
    "TOKEN_S32",
    "TOKEN_S64",
    "TOKEN_F32",
    "TOKEN_F64",
    "TOKEN_BOOL",
    "TOKEN_OPEN_SQUIGGLY_BRACE",
    "TOKEN_CLOSE_SQUIGGLY_BRACE",
    "TOKEN_OPEN_PAREN",
    "TOKEN_CLOSE_PAREN",
    "TOKEN_OPEN_SQUARE_BRACE",
    "TOKEN_CLOSE_SQUARE_BRACE",
    "TOKEN_STRUCT_KEYWORD",
    "TOKEN_TYPEDEF_KEYWORD",
    "TOKEN_UNSIGNED_KEYWORD",
    "TOKEN_STRUCT_NAME",
    "TOKEN_START_COMMENT",
    "TOKEN_NEWLINE",
    "TOKEN_CARRIAGE_RETURN",
    "TOKEN_FUNCTION_NAME"
};

typedef struct {
    String buffer;
    u64 count;
} Tokenizer;

typedef struct {
    String string;
    enum TokenType type;
} Token;

bool is_whitespace(char character) {
    // TODO(ali): There are probably more that we need to handle.
    if (character == ' ' || character == '\t') {
        return true;
    }
    else {
        return false;
    }
}

bool is_newline(char character) {
    return (character == '\r' || character == '\n') ? true : false;
}

bool is_punctuation(char character) {
    if (character == ';' ||
            character == ':' ||
            character == ',' ||
            character == '{' ||
            character == '}' ||
            character == '[' ||
            character == ']' ||
            character == '(' ||
            character == ')' ||
            character == '*')
    {
        return true;
    }
    else {
        return false;
    }
}

Token get_next_token(Tokenizer* tokenizer) {
    if (tokenizer->count >= tokenizer->buffer.len-1) {
        return (Token) {
            .string = NULL_STRING,
                .type = TOKEN_EOF
        };
    }

    u64 token_start_pos, token_end_pos;
    while (is_whitespace(tokenizer->buffer.string[tokenizer->count]) &&
            !is_newline(tokenizer->buffer.string[tokenizer->count]) &&
            !is_punctuation(tokenizer->buffer.string[tokenizer->count]) &&
            tokenizer->count < tokenizer->buffer.len)
    {
        tokenizer->count++;
    }

    // NOTE(ali): If last word contained a punctuation mark, token_end_pos stops there
    //            so token_start_pos now will be at the punctuation now.
    token_start_pos = tokenizer->count;
    if (is_punctuation(tokenizer->buffer.string[token_start_pos]) || 
            is_newline(tokenizer->buffer.string[token_start_pos])) {
        tokenizer->count++;
        switch (tokenizer->buffer.string[token_start_pos]) {
            case ';':  return (Token) { strlit(";"),  TOKEN_SEMICOLON };
            case ':':  return (Token) { strlit(":"),  TOKEN_COLON };
            case ',':  return (Token) { strlit(","),  TOKEN_COMMA };
            case '{':  return (Token) { strlit("{"),  TOKEN_OPEN_SQUIGGLY_BRACE };
            case '}':  return (Token) { strlit("}"),  TOKEN_CLOSE_SQUIGGLY_BRACE };
            case '[':  return (Token) { strlit("["),  TOKEN_OPEN_SQUARE_BRACE };
            case ']':  return (Token) { strlit("]"),  TOKEN_CLOSE_SQUARE_BRACE };
            case '(':  return (Token) { strlit("("),  TOKEN_OPEN_PAREN };
            case ')':  return (Token) { strlit(")"),  TOKEN_CLOSE_PAREN };
            case '*':  return (Token) { strlit("*"),  TOKEN_ASTERIX };
            case '\r': return (Token) { strlit("\r"), TOKEN_CARRIAGE_RETURN };
            case '\n': return (Token) { strlit("\n"), TOKEN_NEWLINE };
        };
    }

    while (!is_whitespace(tokenizer->buffer.string[tokenizer->count]) &&
            !is_punctuation(tokenizer->buffer.string[tokenizer->count]) &&
            !is_newline(tokenizer->buffer.string[tokenizer->count]) &&
            tokenizer->count < tokenizer->buffer.len)
    {
        tokenizer->count++;
    }

    token_end_pos = tokenizer->count;

    String token_string = str_range(tokenizer->buffer, token_start_pos, token_end_pos);

    s64 start_comment_substring_index = str_find_first_index(token_string, strlit("//"));
    if (start_comment_substring_index != -1) {
        tokenizer->count = token_start_pos+start_comment_substring_index+1;
        return (Token) { strlit("//"), TOKEN_START_COMMENT };
    }

    Token token = { .string = token_string };

    if (str_cmp(token_string, strlit("typedef"))) {
        token.type = TOKEN_TYPEDEF_KEYWORD;
    }
    else if (str_cmp(token_string, strlit("struct"))) {
        token.type = TOKEN_STRUCT_KEYWORD;
    }
    else if (str_cmp(token_string, strlit("unsigned"))) {
        token.type = TOKEN_UNSIGNED_KEYWORD;
    }
    else if (str_cmp(token_string, strlit("u8"))) {
        token.type = TOKEN_U8;
    }
    else if (str_cmp(token_string, strlit("u16"))) {
        token.type = TOKEN_U16;
    }
    else if (str_cmp(token_string, strlit("u32"))) {
        token.type = TOKEN_U32;
    }
    else if (str_cmp(token_string, strlit("u64"))) {
        token.type = TOKEN_U64;
    }
    else if (str_cmp(token_string, strlit("s8"))) {
        token.type = TOKEN_S8;
    }
    else if (str_cmp(token_string, strlit("s16"))) {
        token.type = TOKEN_S16;
    }
    else if (str_cmp(token_string, strlit("s32"))) {
        token.type = TOKEN_S32;
    }
    else if (str_cmp(token_string, strlit("s64"))) {
        token.type = TOKEN_S64;
    }
    else if (str_cmp(token_string, strlit("f32"))) {
        token.type = TOKEN_F32;
    }
    else if (str_cmp(token_string, strlit("f64"))) {
        token.type = TOKEN_F64;
    }
    else if (str_cmp(token_string, strlit("uint8_t"))) {
        token.type = TOKEN_U8;
    }
    else if (str_cmp(token_string, strlit("uint16_t"))) {
        token.type = TOKEN_U16;
    }
    else if (str_cmp(token_string, strlit("uint32_t"))) {
        token.type = TOKEN_U32;
    }
    else if (str_cmp(token_string, strlit("uint64_t"))) {
        token.type = TOKEN_U64;
    }
    else if (str_cmp(token_string, strlit("int8_t"))) {
        token.type = TOKEN_S8;
    }
    else if (str_cmp(token_string, strlit("int16_t"))) {
        token.type = TOKEN_S16;
    }
    else if (str_cmp(token_string, strlit("int32_t"))) {
        token.type = TOKEN_S32;
    }
    else if (str_cmp(token_string, strlit("int64_t"))) {
        token.type = TOKEN_S64;
    }
    else if (str_cmp(token_string, strlit("bool"))) {
        token.type = TOKEN_BOOL;
    }
    else if (str_cmp(token_string, strlit("int"))) {
        token.type = TOKEN_S32;
    }
    else if (str_cmp(token_string, strlit("float"))) {
        token.type = TOKEN_F32;
    }
    else if (str_cmp(token_string, strlit("double"))) {
        token.type = TOKEN_F64;
    }
    else if (str_cmp(token_string, strlit("long"))) {
        token.type = TOKEN_S64;
    }
    else if (str_cmp(token_string, strlit("char"))) {
        token.type = TOKEN_U8;
    }
    else {
        token.type = TOKEN_IDENTIFIER;
    }

    return token;
}

#define HANDLE_TYPE(TOKEN_TYPE) \
    if (current_token.type == TOKEN_TYPE) { \
        if (prev_token.type == TOKEN_SEMICOLON || prev_token.type == TOKEN_OPEN_SQUIGGLY_BRACE || prev_token.type == TOKEN_NEWLINE) { \
            str_list_append(scratch.arena, &struct_types, current_token.string); \
        } \
        else { \
            StringNode* prev_type = str_list_get_last(&struct_types); \
            if (prev_type != NULL) { \
                String new_type; \
                if (TOKEN_TYPE != TOKEN_ASTERIX && TOKEN_TYPE != TOKEN_OPEN_SQUARE_BRACE && TOKEN_TYPE != TOKEN_CLOSE_SQUARE_BRACE && TOKEN_TYPE != TOKEN_IDENTIFIER) { \
                    new_type = str_append(scratch.arena, prev_type->string, strlit(" ")); \
                    new_type = str_append(scratch.arena, new_type, current_token.string); \
                } \
                else { \
                    new_type = str_append(scratch.arena, prev_type->string, current_token.string); \
                } \
                prev_type->string = new_type; \
            } \
        } \
    }

void parse(Arena* arena, Tokenizer* tokenizer, String output_file_name) {
    Temp scratch = get_scratch(0, 0);
    FILE* output_file = fopen(output_file_name.string, "wb");

    bool in_struct  = false;
    bool in_struct_definition = false;

    bool in_comment = false;

    Token prev_token    = { .type = TOKEN_UNKNOWN };
    Token current_token = { .type = TOKEN_UNKNOWN };
    Token next_token    = get_next_token(tokenizer);

    StringList struct_types  = {};
    StringList struct_values = {};

    String struct_typedef_name = {};
    String struct_name         = {};

    String members_struct_string = strlit("typedef struct {\n\tMetaType type;\n\tString member_name;\n\tu8 offset;\n} StructMembers;\n\n");

    StringList meta_types_list = {};
    StringList members_structs_list = {};

    str_list_append(arena, &meta_types_list, strlit("META_TYPE_u8"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_u16"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_u32"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_u64"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_s8"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_s16"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_s32"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_s64"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_f32"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_f64"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_bool"));
    str_list_append(arena, &meta_types_list, strlit("META_TYPE_u8_ptr"));

    while (current_token.type != TOKEN_EOF) {
        prev_token    = current_token;
        current_token = next_token;
        next_token    = get_next_token(tokenizer);

        if (in_comment &&
                current_token.type != TOKEN_CARRIAGE_RETURN &&
                current_token.type != TOKEN_NEWLINE)
        {
            continue;
        }
        else {
            in_comment = false;
        }

        if (current_token.type == TOKEN_STRUCT_KEYWORD) {
            in_struct = true;
        }
        else if (current_token.type == TOKEN_IDENTIFIER) {
            if (in_struct) {
                if (prev_token.type == TOKEN_STRUCT_KEYWORD) {
                    struct_name = current_token.string;
                }
                else if (!in_struct_definition && prev_token.type == TOKEN_CLOSE_SQUIGGLY_BRACE) {
                    struct_typedef_name = current_token.string;
                } 
                else if (in_struct_definition && next_token.type == TOKEN_SEMICOLON) {
                    str_list_append(scratch.arena, &struct_values, current_token.string);
                }
                else if (in_struct_definition && next_token.type == TOKEN_OPEN_SQUARE_BRACE) {
                    str_list_append(scratch.arena, &struct_values, current_token.string);
                    continue;
                }
            }
        }
        else if (current_token.type == TOKEN_START_COMMENT) {
            in_comment = true;
        }
        else if (current_token.type == TOKEN_OPEN_SQUIGGLY_BRACE) {
            if (in_struct) {
                in_struct_definition = true;
            }
        }
        else if (current_token.type == TOKEN_CLOSE_SQUIGGLY_BRACE) {
            if (in_struct_definition) {
                in_struct_definition = false;
            }
        }
        else if (current_token.type == TOKEN_SEMICOLON) {
            if (in_struct && !in_struct_definition) {
                StringBuilder builder = {};
                str_builder_init(scratch.arena, &builder, KB(20));

                str_builder_append(&builder, strlit("StructMembers Struct_"));
                str_builder_append(&builder, struct_typedef_name);
                str_builder_append(&builder, strlit("[] = {\n"));

                StringArray struct_types_array  = str_list_to_array(scratch.arena, &struct_types);
                StringArray struct_values_array = str_list_to_array(scratch.arena, &struct_values);

                Assert(struct_types_array.count == struct_values_array.count);
                for (u32 i = 0; i < struct_types_array.count; i++) {
                    str_builder_append(&builder, strlit("\t{ "));
                    String meta_enum_string_value = {};
                    String structs_type_string_copy = struct_types_array.strings[i];

                    if (str_substring_exists(structs_type_string_copy, strlit("char*"))) {
                        str_builder_append(&builder, strlit("META_TYPE_u8_ptr"));
                    }
                    else if (structs_type_string_copy.string[structs_type_string_copy.len-1] == '*') {
                        structs_type_string_copy.len -= 1;
                        meta_enum_string_value = str_append(scratch.arena, strlit("META_TYPE_"), structs_type_string_copy);
                        meta_enum_string_value = str_append(scratch.arena, meta_enum_string_value, strlit("_ptr"));
                        printf("meta_enum_string_value: `%.*s`\n", SP(meta_enum_string_value));

                        if (!str_list_string_exists(&meta_types_list, meta_enum_string_value)) {
                            str_list_append(arena, &meta_types_list, meta_enum_string_value);
                            printf("Appended meta_types_list: %.*s\n", SP(meta_enum_string_value));
                        }
                        str_builder_append(&builder, meta_enum_string_value);
                    }
                    else {
                        meta_enum_string_value = str_append(scratch.arena, strlit("META_TYPE_"), structs_type_string_copy);

                        if (!str_list_string_exists(&meta_types_list, meta_enum_string_value)) {
                            str_list_append(arena, &meta_types_list, meta_enum_string_value);
                        }
                        str_builder_append(&builder, meta_enum_string_value);
                    }

                    str_builder_append(&builder, strlit(", strlit(\""));
                    str_builder_append(&builder, struct_values_array.strings[i]);
                    str_builder_append(&builder, strlit("\"), offsetof("));
                    str_builder_append(&builder, struct_typedef_name);
                    str_builder_append(&builder, strlit(", "));
                    str_builder_append(&builder, struct_values_array.strings[i]);
                    str_builder_append(&builder, strlit(") }"));

                    if (i != struct_values_array.count-1) {
                        str_builder_append(&builder, strlit(",\n"));
                    }
                    else {
                        str_builder_append(&builder, strlit("\n"));
                    }
                }

                str_builder_append(&builder, strlit("};\n\n"));


                printf("=> Parsed %.*s\n", SP(struct_typedef_name));

                /* fwrite(builder.buffer, builder.index, sizeof(char), output_file); */
                str_list_append(arena, &members_structs_list, str_builder_copy_string(arena, builder));

                in_struct = false;
                in_struct_definition = false;
                arena_reset(scratch.arena);
                struct_types = (StringList) {};
                struct_values = (StringList) {};
            }
        }

        if (in_struct_definition && (next_token.type != TOKEN_SEMICOLON || current_token.type == TOKEN_CLOSE_SQUARE_BRACE)) {
            HANDLE_TYPE(TOKEN_U8);
            HANDLE_TYPE(TOKEN_U16);
            HANDLE_TYPE(TOKEN_U32);
            HANDLE_TYPE(TOKEN_U64);
            HANDLE_TYPE(TOKEN_S8);
            HANDLE_TYPE(TOKEN_S16);
            HANDLE_TYPE(TOKEN_S32);
            HANDLE_TYPE(TOKEN_S64);
            HANDLE_TYPE(TOKEN_F32);
            HANDLE_TYPE(TOKEN_F64);
            HANDLE_TYPE(TOKEN_UNSIGNED_KEYWORD);
            HANDLE_TYPE(TOKEN_ASTERIX);
            HANDLE_TYPE(TOKEN_OPEN_SQUARE_BRACE);
            HANDLE_TYPE(TOKEN_CLOSE_SQUARE_BRACE);
            HANDLE_TYPE(TOKEN_IDENTIFIER);
        }
    }

    StringBuilder final_output = {};
    str_builder_init(scratch.arena, &final_output, KB(30));

    str_builder_append(&final_output, strlit("typedef enum {\n"));
    StringNode* string_node;
    STR_LIST_FOREACH(&meta_types_list, string_node) {
        str_builder_append(&final_output, strlit("\t"));
        str_builder_append(&final_output, string_node->string);
        str_builder_append(&final_output, strlit(",\n"));
    }

    final_output.index -= 2;
    str_builder_append(&final_output, strlit("\n} MetaType;\n\n"));
    str_builder_append(&final_output, members_struct_string);

    STR_LIST_FOREACH(&members_structs_list, string_node) {
        str_builder_append(&final_output, string_node->string);
    }

    String final_output_string = str_builder_to_string(final_output);
    /* printf("`%.*s`\n", (u32)final_output_string.len, final_output_string.string); */

    fwrite(final_output_string.string, final_output_string.len, sizeof(char), output_file);

    fclose(output_file);
    
    scratch_end(scratch);
}


void print_struct(Arena* arena, StructMembers* members, u64 members_size, Arena* print_arena) {
    StringBuilder builder = {};
    str_builder_init(arena, &builder, KB(1));
    str_builder_append(&builder, strlit("{\n"));

    for (u32 i = 0; i < members_size; i++) {
        StructMembers current_member = members[i];
        str_builder_append(&builder, strlit("\t"));
        str_builder_append(&builder, current_member.member_name);
        str_builder_append(&builder, strlit(": "));

        char* buffer = push_string(arena, KB(30));
        String value_string = { .string = buffer };
        switch (current_member.type) {
            case (META_TYPE_uint64_t): {
                sprintf(buffer, "%lu", *(u64*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_u16): {
                sprintf(buffer, "%hu", *(u16*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_u32): {
                sprintf(buffer, "%u", *(u32*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_u64): {
                sprintf(buffer, "%lu", *(u64*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_s8): {
                sprintf(buffer, "%hhd", *(s8*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_s16): {
                sprintf(buffer, "%hd", *(s16*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_s32): {
                sprintf(buffer, "%d", *(s32*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_s64): {
                sprintf(buffer, "%lu", *(s64*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_f32): {
                sprintf(buffer, "%f", *(f32*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_f64): {
                sprintf(buffer, "%lf", *(f64*)((u8*)print_arena+current_member.offset));
            } break;
            case (META_TYPE_bool): {
                bool value = *(bool*)((u8*)print_arena+current_member.offset);
                if (value) {
                    strcpy(buffer, "true");
                }
                else {
                    strcpy(buffer, "false");
                }
            } break;
            case (META_TYPE_u8_ptr): {
                char* string = *(char**)((u8*)print_arena+current_member.offset);
                memcpy(buffer, string, print_arena->current_offset);
            } break;
            /* case (META_TYPE_uint64_t): { */

            /* } break; */
            /* case (META_TYPE_Arena_ptr): { */

            /* } break; */

        };

        value_string.len = strlen(buffer);

        if (current_member.type == META_TYPE_u8_ptr) {
            str_builder_append(&builder, strlit("\""));
            str_builder_append(&builder, value_string);
            str_builder_append(&builder, strlit("\""));
        }
        else {
            str_builder_append(&builder, value_string);
        }
        str_builder_append(&builder, strlit(",\n"));


    }
    str_builder_append(&builder, strlit("}"));
    printf("%s\n", str_builder_to_cstring(arena, builder));
}

// TODO(ali): Fix the string_array_append function to work with an empty
//            StringArray.
int main(void) {
    global_arena_setup(MB(5), MB(5));

/*     String output_file_name = strlit("metagen/output.h"); */
/*     String string = load_file_into_string(ctx.global_arena, strlit("metagen/arena_copy.h")); */
/*     Tokenizer tokenizer = { string, 0 }; */
/*     parse(ctx.global_arena, &tokenizer, output_file_name); */
/*     printf("Put output in `%.*s`\n", SP(output_file_name)); */

    print_struct(ctx.global_arena, Struct_Arena, ArrayCount(Struct_Arena), ctx.scratch_pool[0]);
    char* thing =  push_string(ctx.scratch_pool[0], 100);
    strcpy(thing, "alsdfjla;sdjfl;asdjfl;sad");
    print_struct(ctx.global_arena, Struct_Arena, ArrayCount(Struct_Arena), ctx.scratch_pool[0]);

    return 0;
}
