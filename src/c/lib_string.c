#include "header/library.h"


void stringcopy(char* dst, char* src) {
    interrupt(0x21, 0x7, dst, src, 0);
}

int stringlength(char* string) {
  int i = 0;
  while (string[i] != '\0'){
    i++;
  }
  return i;
}

bool stringcompare(char* s1, char* s2) {
  int i = 0;
  if (stringlength(s1) == stringlength(s2)){
    while (s1[i] != '\0'){
      if (s1[i] != s2[i]){
        return false;
      }
      i++;
    }
    return true;
  } else {
    return false;
  }
}



