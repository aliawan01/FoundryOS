#!/bin/bash

mkdir -p build
 
CFLAGS="-I../limine/ \
        -ggdb \
        -O0 \
        -Isrc \
        -o foundryos \
        -Wall \
        -Wextra \
        -std=gnu11 \
        -ffreestanding \
        -fno-stack-protector \
        -fno-stack-check \
        -fno-PIC \
        -I../limine \
        -m64 \
        -march=x86-64 \
        -mno-80387 \
        -mno-mmx \
        -mno-sse \
        -mno-sse2 \
        -mno-red-zone \
        -DLIMINE_API_REVISION=3 \
        -mcmodel=kernel"

LDFLAGS="-Wl,-m,elf_x86_64 \
         -Wl,--build-id=none \
         -nostdlib \
         -z max-page-size=0x1000 \
         -T ../linker.ld"

pushd build && gcc ../src/main.c $CFLAGS $LDFLAGS && ../build_iso.sh && popd
