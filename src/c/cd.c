#include "header/library.h"
int main() {
    int i; int j; int sp1;
    byte inode;
    char args1[64];
    // PROGRAM SETUP START
    struct message msgret; 
    struct message forshell;
    byte cwd_id; char cwd[64];
    int nextSeg; char args2[64];

    getGlobalMsg(&msgret);
    stringcopy(cwd, msgret.arg1);
    stringcopy(args2, msgret.arg2);
    inode = msgret.current_directory;
    cwd_id = msgret.current_directory;
    nextSeg = (mod(((msgret.next_program_segment >> 12) - 0x1),0x5) + 0x3) << 12;
    // PROGRAM SETUP END
    // puts("got here"); endl;
    stringcopy(args1, cwd);
    if (stringcompare("..", args2)) {
        sp1 = 0;
        i = 0;
        while (cwd[i] != '\0') {
        if (cwd[i] == '/') {
            sp1 = i;
        }
        i++;
        }
        if (sp1 == 0) {
        if (cwd[1] != '\0') {
            stringcopy(cwd, "/");
        } else;
        }
        else {
        memorycopy(args1, cwd, sp1);
        args1[sp1] = '\0';
        stringcopy(cwd, args1);
        }
    } else if (stringcompare("/", args2)) {
        stringcopy(cwd, "/");
    } else if (args2[0] == '/') {
        stringcopy(cwd, args2);
        if (cwd[stringlength(cwd)-1] == '/') {
        cwd[stringlength(cwd)-1] = '\0';
        }
    } else {
        sp1 = stringlength(cwd);
        if (!stringcompare(cwd,  "/")) {
        stringcopy(&cwd[sp1], "/");
        sp1++;
        }
        stringcopy(&cwd[sp1], args2);
        if (cwd[stringlength(cwd)-1] == '/') {
        cwd[stringlength(cwd)-1] = '\0';
        }
    }
    inode = getindexcwd(cwd);
    if (inode == 0x40) {

        puts("cd: no such file or directory"); endl;
        clearAllMsg(); // clear all return to shell
        executeNextCmd(0x2000, cwd_id, args1);
    } else {
        executeNextCmd(nextSeg, inode, cwd);
    }
}