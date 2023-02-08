#include "header/library.h"
int main() {
    struct node_filesystem node_fs;
    struct node_entry nebuff; int i; 
    // PROGRAM SETUP START
    struct message msgret; 
    struct message clear_msg;
    char cwd[64]; byte cwd_id;
    int nextSeg;

    getGlobalMsg(&msgret);
    stringcopy(cwd, msgret.arg1);
    cwd_id = msgret.current_directory;
    nextSeg = (mod(((msgret.next_program_segment >> 12) - 0x1),0x5) + 0x3) << 12;

    // PROGRAM SETUP END
    // puts("GOT TO LS"); gets(buf);

    readASector(&node_fs, FS_NODE_SECTOR_NUMBER);
    readASector(&node_fs.nodes[32], FS_NODE_SECTOR_NUMBER+1);
    
    // folder search
    
    for (i = 0; i < 64; i++) {
        memorycopy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
        if (nebuff.parent_node_index == cwd_id && nebuff.sector_entry_index == FS_NODE_S_IDX_FOLDER && nebuff.name[0] != '\0') {
        puts("/");puts(nebuff.name); endl;
        }
    }
    for (i = 0; i < 64; i++) {
        memorycopy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
        if (nebuff.parent_node_index == cwd_id && nebuff.sector_entry_index != FS_NODE_S_IDX_FOLDER && nebuff.name[0] != '\0') {
        puts(nebuff.name); endl;
        }
    }
    // clear message
    executeNextCmd(nextSeg, cwd_id, cwd);
}