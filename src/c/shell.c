#include "header/library.h"


int main() {
  char buf[64]; struct message msg[5]; int i; 

  // PROGRAM SETUP START
  struct message msgret; 
  char cwd[64]; byte icwd; 
  int nextSeg; 

  getGlobalMsg(&msgret);
  stringcopy(cwd, msgret.arg1);
  icwd = msgret.current_directory;
  nextSeg = (mod(((msgret.next_program_segment >> 12) - 0x1),0x5) + 0x2) << 12;

  // cleaning msg if error
  clearAllMsg();
  // PROGRAM SETUP END
  
  puts("FujOShi@OS:"); puts(msgret.arg1); puts("$ ");
  gets(buf);
  splitter(buf, msg);
  writeMsgToGlobalSector(&msg);
  executeNextCmd(nextSeg, icwd, cwd);
}