#include "header/library.h"

int main() {
    char filebuf[8192];
    int nextSeg; byte inode; char src[64]; char dst[64];
    struct message global_msg; char cwd[64];
    struct file_metadata msrc;
    enum fs_retcode return_code;
    char fname[64];
    // GATHER INFO GLOBAL
    getGlobalMsg(&global_msg);
    stringcopy(cwd, global_msg.arg1);
    stringcopy(src, global_msg.arg2);
    stringcopy(dst, global_msg.arg3);
    inode = global_msg.current_directory;
    nextSeg = (mod(((global_msg.next_program_segment >> 12) - 0x1),0x5) + 0x3) << 12;
    msrc.buffer = filebuf;
    stringcopy(msrc.node_name, src);
    msrc.parent_index = inode;
    msrc.filesize = 0;
    readFile(&msrc, &return_code);
    if (return_code == FS_SUCCESS) {
        stringcopy(msrc.node_name, dst);
        writeFile(&msrc, &return_code);
        if (return_code == FS_SUCCESS) {
            puts("writing success"); endl;
            executeNextCmd(nextSeg, inode, cwd);
        } else {
            puts("writing fail"); endl;
            clearAllMsg();
            executeNextCmd(0x2000, inode, cwd);
        }
    } else {
        puts("reading fail"); endl;
        clearAllMsg();
        executeNextCmd(0x2000, inode, cwd);
    }
}