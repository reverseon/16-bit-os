#include "header/library.h"

void readASector(byte* buffer, int sector) {
    interrupt(0x21, 0x9, buffer, sector, 0);
}

void writeASector(byte* buffer, int sector) {
    interrupt(0x21, 0xA, buffer, sector, 0);
}

void readFile(struct file_metadata* metadata, enum fs_retcode* retcode) {
    interrupt(0x21, 0xD, metadata, retcode, 0);
}

void writeFile(struct file_metadata* metadata, enum fs_retcode* retcode) {
    interrupt(0x21, 0xE, metadata, retcode, 0);
}
