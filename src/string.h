#pragma once

typedef struct {
    char* string;
    u64 len;
} String;

#define strlit(s) (String) { s, sizeof(s)-1 }
