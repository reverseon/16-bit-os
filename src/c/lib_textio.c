#include "header/library.h"

void puts(char* buf) {
    interrupt(0x21, 0, buf, 0, 0);
}

void gets(char* buf) {
    interrupt(0x21, 1, buf, 0, 0);
}

void clearTheScreen() {
    interrupt(0x21, 0x8, 0, 0, 0);
}