#include "header/library.h"

int main() {
    struct node_filesystem node_fs;
    struct node_entry nebuff; byte inodesrc; byte inodedst;
    int i; int j; bool found; char cwdtemp[64];
    char fsrc[64]; char fdst[64]; byte inode; char cwd[64]; int nextSeg;
    // GLOBAL GATHER
    struct message global_msg;
    getGlobalMsg(&global_msg);
    // INITIALIZE
    stringcopy(cwd, global_msg.arg1);
    stringcopy(fsrc, global_msg.arg2);
    stringcopy(fdst, global_msg.arg3);
    inode = global_msg.current_directory;
    inodesrc = getindexcwd(cwd);
    nextSeg = (mod(((global_msg.next_program_segment >> 12) - 0x1),0x5) + 0x3) << 12;

    found = false;
    readASector(&node_fs, FS_NODE_SECTOR_NUMBER);
    readASector(&node_fs.nodes[32], FS_NODE_SECTOR_NUMBER+1);
    if (fdst[0] == '/') {
        stringcopy(cwdtemp, fdst);
    } else if (fdst[0] == '.' && fdst[1] == '.' && fdst[2] == '/') {
        stringcopy(cwdtemp, cwd);
        stringcopy(cwdtemp + stringlength(cwd), fdst+2);
    } else {
        i = stringlength(cwd);
        stringcopy(cwdtemp, cwd);
        stringcopy(cwdtemp + i, "/");
        stringcopy(cwdtemp + i+1, fdst);
    }
    
    i = 0; j = 0;
    while (cwdtemp[i] != '\0') {
        if (cwdtemp[i] == '/') j = i;
        i++;
    }
    stringcopy(fdst, cwdtemp+j+1);
    if (j == 0) {
        cwdtemp[1] = '\0';
    } else {
        cwdtemp[j] = '\0';
    }
    inodedst = getindexcwd(cwdtemp);
    // printString("cwdtemp: "); printString(cwdtemp); endl;
    // printString("fdst: "); printString(fdst); endl;
    for (i = 0; i < 64; i++) {
        memorycopy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
        if (stringcompare(nebuff.name, fdst) && nebuff.parent_node_index == inodedst) {
        puts("file/folder already exists"); endl;
        clearAllMsg();
        executeNextCmd(0x2000, inode, cwd);
        }
    }
    for (i = 0; i < 64; i++) {
        memorycopy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
        if (stringcompare(nebuff.name, fsrc) && nebuff.parent_node_index == inodesrc) {
        stringcopy(nebuff.name, fdst);
        nebuff.parent_node_index = inodedst;
        memorycopy(&node_fs.nodes[i], &nebuff, sizeof(struct node_entry));
        writeASector(&node_fs, FS_NODE_SECTOR_NUMBER);
        writeASector(&node_fs.nodes[32], FS_NODE_SECTOR_NUMBER+1);
        puts("success move"); endl;
        executeNextCmd(nextSeg, inode, cwd);
        }
    }
    puts("file/folder not found"); endl;
    clearAllMsg();
    executeNextCmd(0x2000, inode, cwd);
}