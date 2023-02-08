#include "header/library.h"

int div(int a, int b) {
  int c = 0;
  while (c * b <= a) c++;
  return c - 1;
}

int mod(int a, int n) {
  while (a >= n) a -= n;
  return a;
}

void printInteger(int n) {
    int temp = n;
    int i;
    int length = 1;
        char integer[5];
        char isNegative = 0;

    if(temp<0){
        isNegative = true;
        temp = temp * -1;
    }

    while(temp>10){
        temp = div(temp, 10);
        length++;
    }

    temp = n;
    if (isNegative) {
        temp = -temp;
    }

    for(i=length-1; i >= 0; i--) {
        integer[i] = mod(temp,10) + '0';
        temp = div(temp, 10);
    }
    integer[length] = 0;

    if(isNegative) puts('-');
    puts(integer);
}

void memorycopy(byte* dest, byte* src, int size) {
    interrupt(0x21, 0xB, dest, src, size);
}

void clearmem(byte* ptr, unsigned int size) {
    interrupt(0x21, 0xC, ptr, size, 0);
}

void splitter(char* buf, struct message* msg) {
    char singleargs[64];
    char parsed[5*64];
    int i;
    int j;
    int argcount;
    int walker; 
    int cmdcount;

    for (i = 0; i < 5; i++) {
        clearmem(&msg[i], 512);
    }
    cmdcount = 0;
    stringcopy(singleargs, "");
    i = 0;
    walker = 0;
    while (i < 5*64 && cmdcount < 5 && buf[i] != '\0') {
        if (buf[i] == ' ' && buf[i+1] == ';' && buf[i+2] == ' ') {
            parsed[walker + cmdcount*64] = '\0';
            cmdcount++;
            i+=3;
            walker = 0;
        }
        else {
            parsed[walker + cmdcount*64] = buf[i];
            i++;
            walker++;
        }
    }
    parsed[walker + cmdcount*64] = '\0';
    for (i = 0; i <= cmdcount; i++) {
        argcount = 0;
        walker = 0;
        j = 0;
        stringcopy(singleargs, parsed + i*64);
        stringcopy(msg[i].arg1, "");
        stringcopy(msg[i].arg2, "");
        stringcopy(msg[i].arg3, "");
        while(singleargs[j] != '\0') {
            if (singleargs[j] == ' ') {
                if (argcount == 0) {
                    msg[i].arg1[walker] = '\0';
                }
                else if (argcount == 1) {
                    msg[i].arg2[walker] = '\0';
                }
                else if (argcount == 2) {
                    msg[i].arg3[walker] = '\0';
                }
                argcount++;
                walker = 0;
                j++;
            }
            else {
                if (argcount == 0) {
                    msg[i].arg1[walker] = singleargs[j];
                }
                else if (argcount == 1) {
                    msg[i].arg2[walker] = singleargs[j];
                }
                else if (argcount == 2) {
                    msg[i].arg3[walker] = singleargs[j];
                }
                walker++;
                j++;
            }
        }
        if (argcount == 0) {
            msg[i].arg1[walker] = '\0';
        }
        else if (argcount == 1) {
            msg[i].arg2[walker] = '\0';
        }
        else if (argcount == 2) {
            msg[i].arg3[walker] = '\0';
        }
    }
}

void getGlobalMsg(struct message* msg) {
    readASector(msg, MSG_RET_SECT);
}

void writeGlobalMsg(struct message* msg) {
    writeASector(msg, MSG_RET_SECT);
}

void execProgram(struct file_metadata* file, int segment) {
    interrupt(0x21, 0x6, file, segment, 0);
}

byte getindexcwd(char* cwd) {
    struct node_filesystem node_fs;
    struct node_entry nebuff;
    char str1[64]; int it; int i; int depth; int startslash; int endslash; byte prevparidx;
    bool found = false;
    it = 0; startslash = 0; endslash = 0; depth = 0; prevparidx = 0xFF;
    readASector(&node_fs, FS_NODE_SECTOR_NUMBER);
    readASector(&node_fs.nodes[32], FS_NODE_SECTOR_NUMBER+1);

    while (cwd[it] != '\0') {
        if (cwd[it] == '/') {
        if (depth == 0 && cwd[it+1] == '\0') {
            return FS_NODE_P_IDX_ROOT;
        } else if (depth == 0) {
            startslash = it;
        } else {
            endslash = it;
            memorycopy(str1, &cwd[startslash+1], endslash-startslash-1); // string is target
            str1[endslash-startslash-1] = '\0';
            for (i = 0; i < 64; i++) {
            memorycopy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
                if (stringcompare(nebuff.name, str1) && nebuff.parent_node_index == prevparidx && nebuff.sector_entry_index == FS_NODE_S_IDX_FOLDER) {
                prevparidx = i;
                break;
                }
            }
            startslash = it;
        }
        depth++;
        }
        it++;
    }
    endslash = it;
    memorycopy(str1, &cwd[startslash+1], endslash-startslash); // string is target
    str1[endslash-startslash] = '\0';
    for (i = 0; i < 64; i++) {
        memorycopy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
        if (stringcompare(nebuff.name, str1) && nebuff.parent_node_index == prevparidx && nebuff.sector_entry_index == FS_NODE_S_IDX_FOLDER) {
            prevparidx = i;
            found = true;
            break;
        }
    }
    if (found) {
        return prevparidx;
    } else {
        return 0x40;
    }
}

void writeMsgToGlobalSector(struct message* msg) {
    writeASector(&msg[0], MSG_1_SECT);
    writeASector(&msg[1], MSG_2_SECT);
    writeASector(&msg[2], MSG_3_SECT);
    writeASector(&msg[3], MSG_4_SECT);
    writeASector(&msg[4], MSG_5_SECT);
}

void executeNextCmd(int nextSeg, byte icwd, char* cwd) {

    char filebuf[8192];
    int i; bool isthereacmd;
    char filename[64];
    struct message msghldr;

    struct message msgglobal; enum fs_retcode ret;
    struct file_metadata file;
    file.buffer = filebuf;

    // GLOBAL MSG OVERWRITE
    getGlobalMsg(&msgglobal);
    stringcopy(msgglobal.arg1, cwd);
    msgglobal.current_directory = icwd;
    msgglobal.next_program_segment = nextSeg;

    isthereacmd = false;
    for (i = MSG_1_SECT; i <= MSG_5_SECT; i++) {
        readASector(&msghldr, i);
        if (msghldr.arg1[0] != '\0') {
            isthereacmd = true;
            // puts("hello there"); endl;
            // ARGUMENT GATHER
            stringcopy(filename, msghldr.arg1);
            stringcopy(msgglobal.arg2, msghldr.arg2);
            stringcopy(msgglobal.arg3, msghldr.arg3);
            msgglobal.other[0] = i;

            // msg cleanup
            clearmem(&msghldr, sizeof(struct message));
            writeASector(&msghldr, i);
            // BIN CHECKUP
            stringcopy(file.node_name, filename);
            file.parent_index = 0x0; // BIN
            // puts("bef sus"); endl;
            readFile(&file, &ret);
            // puts("afsus"); endl;
            if (ret == FS_SUCCESS) {
                writeGlobalMsg(&msgglobal);
                jumpTo(nextSeg, 0x0, filename);
                break;
            } 
            // LOCAL CHECKUP
            if (filename[0] == '.' && filename[1] == '/') {
                stringcopy(file.node_name, filename+2); // ./filename
                file.parent_index = icwd;
                readFile(&file, &ret);
                if (ret == FS_SUCCESS) {
                    writeGlobalMsg(&msgglobal);
                    jumpTo(nextSeg, icwd, file.node_name);
                    break;
                }
            }
            puts("Command Invalid"); endl;
            isthereacmd = false;
            break;
        }
    }
    if (!isthereacmd) { // back to shell
        stringcopy(msgglobal.arg2, "");
        stringcopy(msgglobal.arg3, "");
        msgglobal.other[0] = -1;
        msgglobal.next_program_segment = 0x2000;
        writeGlobalMsg(&msgglobal);
        jumpTo(0x2000, 0x0, "shell");
    }
}

void clearAllMsg() {
    struct message msghldr;
    int i;
    for (i = MSG_1_SECT; i <= MSG_5_SECT; i++) {
        readASector(&msghldr, i);
        clearmem(&msghldr, sizeof(struct message));
        writeASector(&msghldr, i);
    }
}

void jumpTo(int nextSeg, byte parentinode, char* filename) {
    struct file_metadata nextProgram;
    stringcopy(nextProgram.node_name, filename);
    nextProgram.parent_index = parentinode;
    execProgram(&nextProgram, nextSeg);
}