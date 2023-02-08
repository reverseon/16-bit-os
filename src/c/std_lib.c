#include "header/std_type.h"
#include "header/std_lib.h"


// Opsional : Tambahkan implementasi kode C

int div(int a, int b) {
  int c = 0;
  while (c * b <= a) c++;
  return c - 1;
}

int mod(int a, int n) {
  while (a >= n) a -= n;
  return a;
}

void memcpy(byte *dest, byte *src, unsigned int n) {
  int i;
  for (i = 0; i < n; i++){
    dest[i] = src[i];
  }
}

unsigned int strlen(char *string) {
  int i = 0;
  while (string[i] != '\0'){
    i++;
  }
  return i;
}
// Mengembalikan panjang suatu null terminated string

bool strcmp(char *s1, char *s2) {
  int i = 0;
  if (strlen(s1) == strlen(s2)){
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
// Mengembalikan true jika string sama

void strcpy(char *dst, char *src) {
    char *ptr = dst;
 
    // copy the C-string pointed by source into the array
    // pointed by destination
    while (*src != '\0')
    {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';
}
// Mengcopy n bytes yang ditunjuk src ke dest

void clear(byte *ptr, unsigned int n) {
  int i;
  for (i = 0; i < n; i++) ptr[i] = 0x0;
}