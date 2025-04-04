#include <limine.h>
#include <util.h>
#include <string.h>
#include "drivers/serial.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;


void* memcpy(void* dest, const void* src, u64 n) {
    u8 *pdest = (u8 *)dest;
    const u8 *psrc = (const u8 *)src;

    for (u64 i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, u64 n) {
    u8 *p = (u8 *)s;

    for (u64 i = 0; i < n; i++) {
        p[i] = (u8)c;
    }

    return s;
}

void* memmove(void* dest, const void* src, u64 n) {
    u8 *pdest = (u8 *)dest;
    const u8 *psrc = (const u8 *)src;

    if (src > dest) {
        for (u64 i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (u64 i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, u64 n) {
    const u8 *p1 = (const u8 *)s1;
    const u8 *p2 = (const u8 *)s2;

    for (u64 i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}


bool framebuffer_draw_pixel(struct limine_framebuffer* framebuffer, u32 x, u32 y, u64 color) {
    if (x > framebuffer->width || y > framebuffer->height) {
        return false;
    }

    u32* screen = framebuffer->address;
    u32 pitch_per_pixel = framebuffer->pitch/(framebuffer->bpp/8);
    screen[pitch_per_pixel*y+x] = color;
    return true;
}

void main(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];

    for (u32 i = 20; i < 800; i++) {
        framebuffer_draw_pixel(framebuffer, i, 40,  0xFFFFFF);
        framebuffer_draw_pixel(framebuffer, i, 500, 0xFFFFFF);
    }

    for (u32 i = 40; i < 500; i++) {
        framebuffer_draw_pixel(framebuffer, 20, i, 0xFFFFFF);
        framebuffer_draw_pixel(framebuffer, 800, i, 0xFFFFFF);
    }

    for (u32 y = 41; y < 500; y++) {
        for (u32 x = 21; x < 800; x++) {
            framebuffer_draw_pixel(framebuffer, x, y, 0xFF00FF);
        }
    }

    serial_init();

    /* print_log(strlit("hello world\n")); */

    hcf();
}
