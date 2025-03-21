#include "../util.h"
#include "../string.h"
#include "serial.h"

void outb(u16 port, u8 value) {
    asm(
            "outb %0, %1"
            : 
            : "Nd" (port), "a" (value)
            : "memory"
    );
}

u8 inb(u16 port) {
    u8 value;
    asm(
            "inb %0, %1"
            : "=a" (value)
            : "Nd" (port)
            : "memory"
    );

    return value;
}

int serial_init() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   if(inb(PORT + 0) != 0xAE) {
      return 1;
   }

   outb(PORT + 4, 0x0F);
   return 0;
}

void print_log(String string) {
    for (u64 i = 0; i < string.len; i++) {
        outb(PORT, string.string[i]);
    }
}
