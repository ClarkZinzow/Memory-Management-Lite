#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "mem.h"

#define MAGIC 12345
#define MAX_HEADER_SIZE 32
#define MAX_BITMAP_SIZE 65536

int bitmap[MAX_BITMAP_SIZE];
int bits;

void *head = NULL;
void *base = NULL;
int numCallsInit = 0;
int pageSize;
int numAllocs = 0;
/*
int main() {
  int i = 0;   
   assert(Mem_Init(4096) == 0);
   while(Mem_Alloc(16) != NULL)
      ++i;
   printf("%d", i);
   exit(0);
}
*/

int Mem_Init(int sizeOfRegion) {
  int fd;
  int padSize;

  if(numCallsInit > 0 || sizeOfRegion <= 0)
    return -1;

  pageSize = getpagesize();
  padSize = sizeOfRegion % pageSize;
  padSize = (pageSize - padSize) % pageSize;
  sizeOfRegion += padSize;

  fd = open("/dev/zero", O_RDWR);
  if(fd == -1)
    return -1;
  head = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(head == MAP_FAILED)
    return -1;
  base = head;
  bits = sizeOfRegion / 16;
  close(fd);
  numCallsInit++;
  return 0;
  
}

void* Mem_Alloc(int size) {
  if(size != 16)
    return NULL;

  if(numAllocs >= 253 && bits < 512)
    return NULL;

  void *memPtr = NULL;
  int i;
  for(i = 0; i < bits; i++) {
    if(bitmap[i] == 0) {
      memPtr = head;
      bitmap[i] = 1;
      break;
    }
    head += 16;
  }
  if(!memPtr)
    return NULL;
  numAllocs++;

  return (void *) memPtr;
}

int Mem_Free(void* ptr) {
  int index;  

  if(!ptr)
    return -1;

  if(ptr < base || ptr > base + bits * 16 || (ptr - base) % 16 != 0)
    return -1;

  index = (ptr - base) / 16;

  bitmap[index] = 0;

  return 0;
  
}

int Mem_Available() {
  int numBytesAvailable = 0;
  int i = 0;
  for(i = 0; i < bits; i++) {
    if(bitmap[i] == 0) {
      numBytesAvailable++;
    }
  }
  return numBytesAvailable;
}

void Mem_Dump() {
  
}
