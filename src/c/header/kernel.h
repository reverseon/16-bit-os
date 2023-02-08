
extern void putInMemory(int segment, int address, char character);
extern void launchProgram(int segment);
void executeProgram(struct file_metadata *metadata, int segment);
void makeInterrupt21();
void handleInterrupt21(int AX, int BX, int CX, int DX);
void fillKernelMap();


// TODO : Implementasikan
void printString(char *string);
void readString(char *string);
void clearScreen();

void writeSector(byte *buffer, int sector_number);
void readSector(byte *buffer, int sector_number);

void splashScreen();

void write(struct file_metadata *metadata, enum fs_retcode *return_code);
void read(struct file_metadata *metadata, enum fs_retcode *return_code);

void shell();

byte getinodecwd(char* cwd);

void respcode(enum fs_retcode return_code);

//kepentingan shell

// void cdHandler(char *path);
// void lsHandler(char* cwd);
// void mvHandler(char *srcpath, char *dstpath);
// void mkdirHandler(char* cwd, char* path, unsigned int pathsize, unsigned int cwdsize);
// void catHandler(char *path);
// void cpHandler(char *srcpath, char *dstpath);
