#pragma once

typedef struct {
    char* string;
    u64 len;
} String;

#define str_lit(s) (String) { s, sizeof(s)-1 }
