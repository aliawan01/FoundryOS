#pragma once

#define PORT (u16)0x3F8

void outb(u16 port, u8 value);
u8   inb(u16 port);
int  serial_init();
void print_log(String string);
