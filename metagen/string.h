#pragma once

typedef struct {
    char* string;
    u64 len;
} String;

typedef struct StringNode {
    String string;
    struct StringNode* child;
} StringNode;

typedef struct {
    StringNode* node;
    u64 count;
} StringList;

typedef struct {
    String* strings;
    u64 count;
} StringArray;

typedef struct {
    char* buffer;
    u64   index;
    u64   len;
} StringBuilder;

#define strlit(s) (String) {s, sizeof(s)-1}
#define NULL_STRING (String) {NULL, 0}
#define SP(s) (u32)s.len, s.string

#define STR_LIST_FOREACH(array, elem) for ((elem) = (array)->node; (elem) != NULL; (elem) = (elem)->child)
#define STR_LIST_FOREACH_VALID_CHILD(array, elem) for ((elem) = (array)->node; (elem)->child != NULL; (elem) = (elem)->child)

String      str_append(Arena* arena, String originalString, String appendString); 
String      str_copy(Arena* arena, String string);
char*       str_to_cstring(Arena* arena, String string); 
String      str_range(String string, uint64_t startPos, uint64_t endPos); 
bool        str_cmp(String firstString, String secondString); 
s64         str_find_first_index(String originalString, String substring);
bool        str_substring_exists(String originalString, String substring); 
String      str_trim_whitespace(String string); 
StringList  str_sep_on_whitespace(Arena* arena, String string); 
void        str_builder_init(Arena* arena, StringBuilder* builder, uint64_t max_size);
bool        str_builder_append(StringBuilder* builder, String appendString); 
bool        str_builder_remove(StringBuilder* builder, uint64_t removeLen); 
String      str_builder_to_string(StringBuilder builder); 
String      str_builder_copy_string(Arena* arena, StringBuilder builder); 
char*       str_builder_to_cstring(Arena* arena, StringBuilder builder); 
void        str_list_append(Arena* arena, StringList* array, String string);
StringNode* str_list_get_last(StringList* array);
StringArray str_list_to_array(Arena* arena, StringList* array);
bool        str_list_string_exists(StringList* array, String searchString);
