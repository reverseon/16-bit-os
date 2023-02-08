#include "std_type.h"
#include "interrupt.h"
#include "filesystem.h"
#define endl puts("\r\n")
#define sp puts(" ")
#define MSG_1_SECT 0x190
#define MSG_2_SECT 0x191
#define MSG_3_SECT 0x192
#define MSG_4_SECT 0x193
#define MSG_5_SECT 0x194
#define MSG_RET_SECT 0x198

void readASector(byte* buffer, int sector);
void writeASector(byte* buffer, int sector);
void clearTheScreen();
void puts(char* buf);
void gets(char* buf);
void stringcopy(char* dst, char* src);
bool stringcompare(char* s1, char* s2);
int stringlength(char* string);
void memorycopy(byte* dest, byte* src, int size);
void splitter(char* buf, struct message* msg);
byte getindexcwd(char* cwd);
void getGlobalMsg(struct message* msg);
void setGlobalMsg(struct message* msg);
void execProgram(struct file_metadata* file, int segment);
int div(int a, int b);
int mod(int a, int n);
void printInteger(int n);
void executeNextCmd(int nextSeg, byte icwd, char* cwd);
void writeMsgToGlobalSector(struct message* msg);
void jumpTo(int nextSeg, byte icwd, char* cwd, char* filename);
void readFile(struct file_metadata* metadata, enum fs_retcode retcode);
void writeFile(struct file_metadata* metadata, enum fs_retcode retcode);
void clearAllMsg();
void clearmem(byte* ptr, unsigned int size);