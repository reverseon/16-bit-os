// Kode kernel
// PENTING : FUNGSI PERTAMA YANG DIDEFINISIKAN ADALAH main(),
//   cek spesifikasi untuk informasi lebih lanjut

// TODO : Tambahkan implementasi kode C

// DEVELOPER NOTES
// TWFhZiBrYWsga2FsYXUgYmVyYW50YWthbiwgYWt1IG5nZXJqYWluIGluaSBzZW5kaXJpYW4uIEdoZWJ5b24gc2FtYSBBZHprYSBBRksgOig=

#define endl interrupt(0x21, 0x0, "\r\n", 0, 0)
#define sp interrupt(0x21, 0x0, " ", 0, 0)
#include "header/std_type.h"
#include "header/std_lib.h"
#include "header/kernel.h"
#include "header/interrupt.h"
#include "header/filesystem.h"
int pagenum = 0;
int rowt = 25;
int colt = 80;



int main() {
  // INITIALIZER
  struct file_metadata sh; struct message global_payload;
  fillKernelMap();
  makeInterrupt21();

  global_payload.current_directory = getinodecwd("/");
  strcpy(global_payload.arg1, "/");
  global_payload.next_program_segment = 0x2000;
  writeSector(&global_payload, 0x198);
  strcpy(sh.node_name, "shell");
  sh.parent_index = 0x0; // bin

  clearScreen();
  endl; endl;
  executeProgram(&sh, 0x2000);
  // KERNEL STAY
}

void executeProgram(struct file_metadata *metadata, int segment) {
    enum fs_retcode fs_ret;
    byte buf[8192];
    metadata->buffer = buf;
    read(metadata, &fs_ret);
    if (fs_ret == FS_SUCCESS) {
      int i = 0;
      for (i = 0; i < 8192; i++) {
        if (i < metadata->filesize)
          putInMemory(segment, i, metadata->buffer[i]);
        else
          putInMemory(segment, i, 0x00);
      }
      launchProgram(segment);
    } else
    printString("exec: file not found\r\n");
}

void handleInterrupt21(int AX, int BX, int CX, int DX) {
	byte AL,AH;
  int holder;
	AL = AX & 0xFF;
	AH = AX >> 8;
  switch (AX) {
      case 0x0:
          printString(BX);
          break;
      case 0x1:
          readString(BX);
          break;
      case 0x2:
          readSector(BX, CX);
          break;
      case 0x3:
          writeSector(BX, CX);
          break;
      case 0x4:
          read(BX, CX);
          break;
      case 0x5:
          write(BX, CX);
          break;
      case 0x6:
          executeProgram(BX, CX);
          break;
      case 0x7:
          strcpy(BX, CX);
          break;
      case 0x8:
          clearScreen();
          break;
      case 0x9:
          readSector(BX, CX);
          break;
      case 0xA:
          writeSector(BX, CX);
          break;
      case 0xB:
          memcpy(BX, CX, DX);
          break;
      case 0xC:
          clear(BX, CX);
          break;
      case 0xD:
          read(BX, CX);
          break;
      case 0xE:
          write(BX, CX);
          break;
      default:
          printString("INVINT");
  }

}

void printString(char* string) {
	int i = 0;
	while (string[i] != '\0') { // 0x0 == '\0'
    if (string[i] == '\n' && string[i-1] != '\r') {
      interrupt(0x10, 0xE00 + '\r', pagenum, 0, 0);
      interrupt(0x10, 0xE00+string[i], pagenum, 0, 0);
    } else if (string[i] == 0x22) {
      interrupt(0x10, 0xE00 + '"', pagenum, 0, 0);
    } else {
      interrupt(0x10, 0xE00+string[i], pagenum, 0, 0);
    }
		
		i++;
	}
}

void readString(char* toread) {
    int count = 0;
    char c;
    while (true) {
        c = interrupt(0x16, 0, 0, 0, 0);
        if (c == '\r') {
            interrupt(0x10, 0xE00+'\r', pagenum, 0, 0);
            interrupt(0x10, 0xE00+'\n', pagenum, 0, 0);
            toread[count] = '\0';
            break;
        } else if (c == '\b') {
            if (count > 0) {
                toread[count] = '\0';
                interrupt(0x10, 0xE00+'\b', pagenum, 0, 0);
                interrupt(0x10, 0xE00+' ', pagenum, 0, 0);
                interrupt(0x10, 0xE00+'\b', pagenum, 0, 0);
                count--;
            }
        } else if (c == 0x2B) {
            interrupt(0x10, 0xE00+0x5F, pagenum, 0, 0);
            toread[count] = 0x5F;
            count++;
        } else if (c == 0x7E) {
            interrupt(0x10, 0xE00+0x2B, pagenum, 0, 0);
            toread[count] = 0x2B;
            count++;
        } else {
            interrupt(0x10, 0xE00+c, pagenum, 0, 0);
            toread[count] = c;
            count++;
        }
    }
}

void clearScreen() {
    int i = 0;
    int j = 0;
    for (i = 0; i < 25; i++) {
      for (j = 0; j < 80; j++) {
        int where = 80*i+j;
        putInMemory(0xB000, 0x8000 + 2*where, 0x0);
        putInMemory(0xB000, 0x8001 + 2*where, 0xF);
      }
    }
    interrupt(0x10, 0x200, 0, 0, 0);
}

byte getinodecwd(char* cwd) {
  struct node_filesystem node_fs;
  struct node_entry nebuff;
  char str1[64]; int it; int i; int depth; int startslash; int endslash; byte prevparidx;
  bool found = false;
  it = 0; startslash = 0; endslash = 0; depth = 0; prevparidx = 0xFF;
  readSector(&node_fs, FS_NODE_SECTOR_NUMBER);
  readSector(&node_fs.nodes[32], FS_NODE_SECTOR_NUMBER+1);

  while (cwd[it] != '\0') {
    if (cwd[it] == '/') {
      if (depth == 0 && cwd[it+1] == '\0') {
        return FS_NODE_P_IDX_ROOT;
      } else if (depth == 0) {
        startslash = it;
      } else {
        endslash = it;
        memcpy(str1, &cwd[startslash+1], endslash-startslash-1); // string is target
        str1[endslash-startslash-1] = '\0';
        for (i = 0; i < 64; i++) {
           memcpy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
          //  printString(nebuff.name); sp; printString(str1); endl;
            if (strcmp(nebuff.name, str1) && nebuff.parent_node_index == prevparidx && nebuff.sector_entry_index == FS_NODE_S_IDX_FOLDER) {
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
  memcpy(str1, &cwd[startslash+1], endslash-startslash); // string is target
  str1[endslash-startslash] = '\0';
  for (i = 0; i < 10; i++) {
      memcpy(&nebuff, &node_fs.nodes[i], sizeof(struct node_entry));
      // printString(nebuff.name); sp; printString(str1); endl;
      if (strcmp(nebuff.name, str1) && nebuff.parent_node_index == prevparidx && nebuff.sector_entry_index == FS_NODE_S_IDX_FOLDER) {
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

void readSector(byte *buffer, int sector_number){
    int sector_read_count = 0x01;
    int cylinder, sector;
    int head, drive;

    cylinder = div(sector_number, 36) << 8; // CH
    sector   = mod(sector_number, 18) + 1;  // CL

    head  = mod(div(sector_number, 18), 2) << 8; // DH
    drive = 0x00;                                // DL

    interrupt(
        0x13,                       // Interrupt number
        0x0200 | sector_read_count, // AX
        buffer,                     // BX
        cylinder | sector,          // CX
        head | drive                // DX
    );

}
void writeSector(byte *buffer, int sector_number){
    int sector_read_count = 0x01;
    int cylinder, sector;
    int head, drive;

    cylinder = div(sector_number, 36) << 8; // CH
    sector   = mod(sector_number, 18) + 1;  // CL

    head  = mod(div(sector_number, 18), 2) << 8; // DH
    drive = 0x00;                                // DL

    interrupt(
        0x13,                       // Interrupt number
        0x0300 | sector_read_count, // AX
        buffer,                     // BX
        cylinder | sector,          // CX
        head | drive                // DX
    );
}

void fillKernelMap() {
  struct map_filesystem map_fs_buffer;
  int i;

  readSector(&map_fs_buffer, FS_MAP_SECTOR_NUMBER);
  // sysreserve
  for (i = 0; i < 32; i++) {
    map_fs_buffer.is_filled[i] = true;
  }
  // sysfile
  for (i = 32; i < 256; i++) {
    map_fs_buffer.is_filled[i] = false;
  }
  // sysres
  for (i = 256; i < 512; i++) {
    map_fs_buffer.is_filled[i] = true;
  }
  writeSector(&map_fs_buffer, FS_MAP_SECTOR_NUMBER);
}

void read(struct file_metadata *metadata, enum fs_retcode *return_code) {
  struct node_filesystem   node_fs_buffer;
  struct sector_filesystem sector_fs_buffer;
  struct sector_entry sebuff;
  struct node_entry nebuff;
  // // Tambahkan tipe data yang dibutuhkan
  int i; int j;
  byte sectorref;


  // // Masukkan filesystem dari storage ke memori buffer
  readSector(&node_fs_buffer, FS_NODE_SECTOR_NUMBER);
  readSector(&node_fs_buffer.nodes[32], FS_NODE_SECTOR_NUMBER+1);
  readSector(&sector_fs_buffer, FS_SECTOR_SECTOR_NUMBER);
  // 1. Cari node dengan nama dan lokasi yang sama pada filesystem.

  metadata->filesize = 0;
  for (i = 0; i < 64; i++) {
    memcpy(&nebuff, &node_fs_buffer.nodes[i], sizeof(struct node_entry));
    if (strcmp(nebuff.name, metadata->node_name) && nebuff.parent_node_index == metadata->parent_index) {
        sectorref = nebuff.sector_entry_index;
        if (sectorref == FS_NODE_S_IDX_FOLDER) {
          *return_code = FS_R_TYPE_IS_FOLDER;
        } else {
            memcpy(&sebuff, &sector_fs_buffer.sector_list[sectorref], sizeof(struct sector_entry));
            j = 0;
            metadata->filesize = 0;
            // printString("whatsector r: "); sp;
            while (sebuff.sector_numbers[j] != 0x0 && j < 16) {
              // uintprint(sebuff.sector_numbers[j]); sp;
              readSector(metadata->buffer + j*512, sebuff.sector_numbers[j]);
              metadata->filesize += 512;
              j++;
            }
            // endl;
            // printString("mtdt info r"); endl;
            // printString(metadata->node_name); endl;
            // uintprint(metadata->parent_index); endl;
            // uintprint(metadata->filesize); endl;
            // uintprint(i); endl;
            // uintprint(sectorref); endl;
            *return_code = FS_SUCCESS;
        }
        return;
      }
  }
  *return_code = FS_R_NODE_NOT_FOUND;
  return;
}



void write(struct file_metadata *metadata, enum fs_retcode *return_code) {
  struct node_filesystem   node_fs_buffer;
  struct sector_filesystem sector_fs_buffer;
  struct map_filesystem    map_fs_buffer;


  // Tambahkan tipe data yang dibutuhkan
  struct node_entry nebuff; struct sector_entry sebuff;
  int i; int j; //loopers
  int fzins; int inode; int isect; int emsect;// multipurpose numbers
  byte pb; byte sb; // multipurpose byte
  bool found;
  // Masukkan filesystem dari storage ke memori
  readSector(&node_fs_buffer, FS_NODE_SECTOR_NUMBER);
  readSector(&node_fs_buffer.nodes[32], FS_NODE_SECTOR_NUMBER+1);
  readSector(&sector_fs_buffer, FS_SECTOR_SECTOR_NUMBER);
  readSector(&map_fs_buffer, FS_MAP_SECTOR_NUMBER);
  if (div(metadata->filesize, 512) * 512 == metadata->filesize) {
    fzins = div(metadata->filesize, 512);
  } else {
    fzins = div(metadata->filesize, 512) + 1;
  }
  // 1. Cari node dengan nama dan lokasi parent yang sama pada node.
  //    Jika tidak ditemukan kecocokan, lakukan proses ke-2.
  //    Jika ditemukan node yang cocok, tuliskan retcode 
  //    FS_W_FILE_ALREADY_EXIST dan keluar. 
  for (i = 0; i < 64; i++) {
    memcpy(&nebuff, &node_fs_buffer.nodes[i], sizeof(struct node_entry));
    if (strcmp(nebuff.name, metadata->node_name) && nebuff.parent_node_index == metadata->parent_index) {
      *return_code = FS_W_FILE_ALREADY_EXIST;
      return;
    }
  }
  // 2. Cari entri kosong pada filesystem node dan simpan indeks.
  //    Jika ada entry kosong, simpan indeks untuk penulisan.
  //    Jika tidak ada entry kosong, tuliskan FS_W_MAXIMUM_NODE_ENTRY
  //    dan keluar.
  found = false;
  for (i = 0; i < 64; i++) {
    memcpy(&nebuff, &node_fs_buffer.nodes[i], sizeof(struct node_entry));
    if (nebuff.name[0] == 0x0) {
      inode = i;
      found = true;
      break;
    }
  }

  if (!found && metadata->filesize == 0) {
    *return_code = FS_W_MAXIMUM_NODE_ENTRY;
    return;
  }
  // 3. Cek dan pastikan entry node pada indeks P adalah folder.
  //    Jika pada indeks tersebut adalah file atau entri kosong,
  //    Tuliskan retcode FS_W_INVALID_FOLDER dan keluar.
  if (metadata->parent_index != 0xFF) {
    memcpy(&nebuff, &node_fs_buffer.nodes[metadata->parent_index], sizeof(struct node_entry));
    if (nebuff.sector_entry_index == 0x0 || nebuff.sector_entry_index != FS_NODE_S_IDX_FOLDER) {
      *return_code = FS_W_INVALID_FOLDER;
      return;
    }
  }
  // 4. Dengan informasi metadata filesize, hitung sektor-sektor 
  //    yang masih kosong pada filesystem map. Setiap byte map mewakili 
  //    satu sektor sehingga setiap byte mewakili 512 bytes pada storage.
  //    Jika empty space tidak memenuhi, tuliskan retcode
  //    FS_W_NOT_ENOUGH_STORAGE dan keluar.
  //    Jika ukuran filesize melebihi 8192 bytes, tuliskan retcode
  //    FS_W_NOT_ENOUGH_STORAGE dan keluar.
  //    Jika tersedia empty space, lanjutkan langkah ke-5.
  if (metadata->filesize > 8192) {
    *return_code = FS_W_NOT_ENOUGH_STORAGE;
    return;
  }

  emsect = 0;
  for (i = 0; i < 512; i++) {
    if (map_fs_buffer.is_filled[i] == 0x0) {
      emsect+=1;
    }
  }

  if (emsect < fzins) {
    *return_code = FS_W_NOT_ENOUGH_STORAGE;
    return;
  }
  // 5. Cek pada filesystem sector apakah terdapat entry yang masih kosong.
  //    Jika ada entry kosong dan akan menulis file, simpan indeks untuk 
  //    penulisan.
  //    Jika tidak ada entry kosong dan akan menulis file, tuliskan
  //    FS_W_MAXIMUM_SECTOR_ENTRY dan keluar.
  //    Selain kondisi diatas, lanjutkan ke proses penulisan.
  found = false;
  for (i = 0; i < 32; i++) {
    memcpy(&sebuff, &sector_fs_buffer.sector_list[i], sizeof(struct sector_entry));
    if (sebuff.sector_numbers[0] == 0x0) {
      isect = i;
      found = true;
      break;
    }
  }

  if (!found && metadata->filesize > 0) {
    *return_code = FS_W_MAXIMUM_SECTOR_ENTRY;
    return;
  }
  // // Penulisan
  // // 1. Tuliskan metadata nama dan byte P ke node pada memori buffer
  // // 2. Jika menulis folder, tuliskan byte S dengan nilai
  // //    FS_NODE_S_IDX_FOLDER dan lompat ke langkah ke-8 
  // // 3. Jika menulis file, tuliskan juga byte S sesuai indeks sector

  if (metadata->filesize == 0) {
    pb = metadata->parent_index;
    sb = FS_NODE_S_IDX_FOLDER;
  } else {
    pb = metadata->parent_index;
    sb = isect;
  }

  strcpy(nebuff.name, metadata->node_name);
  nebuff.parent_node_index = pb;
  nebuff.sector_entry_index = sb;
  memcpy(&node_fs_buffer.nodes[inode], &nebuff, sizeof(struct node_entry));

  // printString("Here: "); printString(metadata->node_name); endl;
  // if (nodeid <= 63) {

  if (metadata->filesize > 0) {
    // 4. Persiapkan variabel j = 0 untuk iterator entry sector yang kosong
    // 5. Persiapkan variabel buffer untuk entry sector kosong
    // 6. Lakukan iterasi berikut dengan kondisi perulangan 
          // (penulisan belum selesai && i = 0..255)
        //    1. Cek apakah map[i] telah terisi atau tidak
    //    2. Jika terisi, lanjutkan ke iterasi selanjutnya / continue
    //    3. Tandai map[i] terisi
    //    4. Ubah byte ke-j buffer entri sector dengan i
    //    5. Tambah nilai j dengan 1
    //    6. Lakukan writeSector() dengan file pointer buffer pada metadata 
    //       dan sektor tujuan i
    //    7. Jika ukuran file yang telah tertulis lebih besar atau sama dengan
    //       filesize pada metadata, penulisan selesai
    
    // 7. Lakukan update dengan memcpy() buffer entri sector dengan 
    //    buffer filesystem sector

    i = 32; // sector reserved for fsys
    j = 0;
    memcpy(&sebuff, &sector_fs_buffer.sector_list[isect], sizeof(struct sector_entry));
    // printString(metadata->buffer); endl; printString("gg"); endl;
    // return;
    // printString("whatsector w: "); sp;
    while (fzins > 0 && i < 256) {
      if (!map_fs_buffer.is_filled[i]) {
        // uintprint(i); sp;
        writeSector(&metadata->buffer[j*512], i);
        sebuff.sector_numbers[j] = i;
        j++;
        map_fs_buffer.is_filled[i] = true;
        fzins -= 1;
      }
      i++;
    }
    // endl;
    memcpy(&sector_fs_buffer.sector_list[isect], &sebuff, sizeof(struct sector_entry));
  }

  // // 8. Lakukan penulisan seluruh filesystem (map, node, sector) ke storage
  // //    menggunakan writeSector() pada sektor yang sesuai
  writeSector(&map_fs_buffer, FS_MAP_SECTOR_NUMBER);
  writeSector(&node_fs_buffer, FS_NODE_SECTOR_NUMBER);
  writeSector(&node_fs_buffer.nodes[32], FS_NODE_SECTOR_NUMBER + 1);
  writeSector(&sector_fs_buffer, FS_SECTOR_SECTOR_NUMBER);
  // // 9. Kembalikan retcode FS_SUCCESS
  *return_code = FS_SUCCESS;
  return;
}

