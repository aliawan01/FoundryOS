#include "util.h"
#include "arena.h"
#include "string.h"

String str_append(Arena* arena, String originalString, String appendString) {
    uint64_t stringLen = originalString.len + appendString.len;
    char* buffer = push_string(arena, stringLen+1);
    memcpy(buffer, originalString.string, originalString.len);

    memcpy(buffer+originalString.len, appendString.string, appendString.len);
    return (String) {buffer, stringLen};
}


String str_copy(Arena* arena, String string) {
    char* string_copy = push_string(arena, string.len+1);
    memcpy(string_copy, string.string, string.len);
    return (String) { string_copy, string.len };
}

char* str_to_cstring(Arena* arena, String string) {
    if (string.len == 0) {
        return "(null)";
    }

    char* cstring = push_string(arena, string.len+1);
    memcpy(cstring, string.string, string.len);
    cstring[string.len] = 0;
    return cstring;
}

String str_range(String string, uint64_t startPos, uint64_t endPos) {
    if (endPos > string.len || startPos > endPos) {
        return (String) { NULL, -1 };
    }

    return (String) { string.string+startPos, endPos-startPos };
}

bool str_cmp(String firstString, String secondString) {
    if (firstString.len != secondString.len) {
        return false;
    }

    for (u64 i = 0; i < firstString.len; i++) {
        if (firstString.string[i] != secondString.string[i]) {
            return false;
        }
    }

    return true;
}

s64 str_find_first_index(String originalString, String substring) {
    if (originalString.len < substring.len) {
        return -1;
    }

    for (u64 i = 0; i < originalString.len-substring.len+1; i++) {
        String originalStringOffset = (String) {originalString.string+i, substring.len};
        if (str_cmp(originalStringOffset, substring)) {
            return i;
        }

    }

    return -1;
}

bool str_substring_exists(String originalString, String substring) {
    if (str_find_first_index(originalString, substring) != -1) {
        return true;
    }
    else {
        return false;
    }
}

String str_trim_whitespace(String string) {
    int startIndex = 0;
    for (u64 i = 0; i < string.len; i++) {
        if (string.string[i] != ' ') {
            startIndex = i;
            break;
        }
    }

    int endIndex = 0;
    for (int i = string.len-1; i > startIndex; i--) {
        if (string.string[i] != ' ') {
            endIndex = i+1;
            break;
        }
    }

    return (String) {string.string+startIndex, endIndex-startIndex};
}

StringList str_sep_on_whitespace(Arena* arena, String string) {
    String trimmedString = str_trim_whitespace(string);

    StringList array = {};
    array.node = push_struct(arena, StringNode);

    StringNode* currentNode = array.node;
    for (u64 i = 0, prevStartPos = 0; i < trimmedString.len; i++) {
        if (trimmedString.string[i] == ' ' || i == trimmedString.len-1) {
            uint64_t stringLen = i-prevStartPos;
            char* buffer = push_string(arena, stringLen);
            memcpy(buffer, trimmedString.string+prevStartPos, stringLen);

            currentNode->string = (String) { buffer, stringLen };
            currentNode->child = push_struct(arena, StringNode);
            currentNode = currentNode->child;
            prevStartPos = i+1;
        }
    }

    return array;
}
void str_builder_init(Arena* arena, StringBuilder* builder, uint64_t max_size) {
    *builder = (StringBuilder) {
        .buffer = push_string(arena, max_size),
        .index = 0,
        .len = max_size
    };
}

bool str_builder_append(StringBuilder* builder, String appendString) {
    if (builder->index + appendString.len > builder->len) {
        return false;
    }

    memcpy(builder->buffer+builder->index, appendString.string, appendString.len);
    builder->index += appendString.len;
    return true;
}

bool str_builder_remove(StringBuilder* builder, uint64_t removeLen) {
    if (builder->index-removeLen < 0) {
        return false;
    }

    builder->index -= removeLen;
    return true;
}

String str_builder_to_string(StringBuilder builder) {
    return (String) { builder.buffer, builder.index };
}

String str_builder_copy_string(Arena* arena, StringBuilder builder) {
    char* stringCopy = push_string(arena, builder.index);
    memcpy(stringCopy, builder.buffer, builder.index);
    return (String) { stringCopy, builder.index };
}


char* str_builder_to_cstring(Arena* arena, StringBuilder builder) {
    char* stringCopy = push_string(arena, builder.index+1);
    memcpy(stringCopy, builder.buffer, builder.index);
    return stringCopy;
}

void str_list_append(Arena* arena, StringList* array, String string) {
    StringNode** node = &array->node;
    while (*node != NULL) {
        node = &(*node)->child;
    }

    *node = push_struct(arena, StringNode);
    (*node)->string = str_copy(arena, string);
    (*node)->child = NULL;
    array->count++;
}

StringNode* str_list_get_last(StringList* array) {
    if (array->count == 0) {
        return NULL;
    }

    StringNode* string_node;
    STR_LIST_FOREACH_VALID_CHILD(array, string_node);
    return string_node;
}

StringArray str_list_to_array(Arena* arena, StringList* array) {
    if (array->count == 0) {
        return (StringArray) { NULL, 0 };
    }

    StringArray string_array = {
        .strings = push_array(arena, String, array->count),
        .count = array->count
    };

    StringNode* string_node;
    int i = 0;
    STR_LIST_FOREACH(array, string_node) {
        string_array.strings[i] = str_copy(arena, string_node->string);
        i++;
    }

    return string_array;
}

bool str_list_string_exists(StringList* array, String search_string) {
    StringNode* string_node;
    STR_LIST_FOREACH(array, string_node) {
        if (str_cmp(string_node->string, search_string)) {
            return true;
        }
    }

    return false;
}
