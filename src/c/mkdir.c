#include "header/library.h"

int main() {
    struct file_metadata metadata;
    enum fs_retcode return_code;
    struct message global_msg; char fname[64]; byte inode; int nextSeg;
    // GATHER INFO GLOBAL
    getGlobalMsg(&global_msg);
    stringcopy(fname, global_msg.arg2);
    inode = global_msg.current_directory;
    nextSeg = (mod(((global_msg.next_program_segment >> 12) - 0x1),0x5) + 0x3) << 12;

    // SETUP END
    metadata.parent_index = inode;
    stringcopy(metadata.node_name, fname);
    metadata.filesize = 0;
    writeFile(&metadata, &return_code);
    if (return_code == FS_SUCCESS) {
        puts("success creating a folder"); endl;
        executeNextCmd(nextSeg, inode, global_msg.arg1);
    } else {
        puts("failed to create a folder"); endl;
        clearAllMsg();
        executeNextCmd(0x2000, inode, global_msg.arg1);
    }
}