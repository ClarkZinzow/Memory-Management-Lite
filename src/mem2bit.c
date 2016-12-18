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

typedef struct __list_t {
  int size;
} list_t;

int bitmap[MAX_BITMAP_SIZE];
int bits;

list_t *head = NULL;
list_t *base = NULL;
int numCallsInit = 0;
int pageSize;
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
  head->size = sizeOfRegion;
  base = head;
  bits = sizeOfRegion / 16;
  close(fd);
  numCallsInit++;
  return 0;
  
}

void* Mem_Alloc(int size) {
  if(size != 16 && size != 80 && size != 256)
    return NULL;

  void *memPtr = NULL;
  int i;
  list_t *tmp = head;
  for(i = 0; i < bits; i++) {
    if(bitmap[i] & 1 == 0) {
      memPtr = tmp;
      if(size == 16) {
        bitmap[i] = 3;
        break;
      }
      else if(size == 80) {
        bitmap[i] = 5;
        break;
      }
      else {
        bitmap[i] = 7;
        break;
      }
    }
    if(bitmap[i] == 3) {
      tmp += 16;
    }
    else if(bitmap[i] == 5) {
      tmp += 80;
    }
    else {
      tmp += 256;
    }
  }
  if(!memPtr)
    return NULL;

  return (void *) memPtr;
}

int Mem_Free(void* ptr) {
  int index;  

  if(!ptr)
    return -1;

  list_t *tmp = (void *) ptr;

  if(ptr < base || ptr > base + bits || (ptr - base) % 16 != 0 || !tmp->size)
    return -1;

  index = base - tmp->size;

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
