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

typedef struct __list_t {
  int size;
  struct __list_t *next;
  struct __list_t *prev;
} list_t;

typedef struct __header_t {
  int size;
  int magic;
} header_t;

list_t *head = NULL;
int numCallsInit = 0;
int pageSize;

/*

int main() {
  assert(Mem_Init(4096) == 0);
   void* ptr[4];
   Mem_Dump();
   printf("\n");
   ptr[0] = Mem_Alloc(8);
   ptr[1] = Mem_Alloc(16);
   Mem_Dump();
printf("\n");
   Mem_Free(ptr[0]);
   Mem_Free(ptr[1]);
   assert(Mem_Free(ptr[0]) == 0);
   assert(Mem_Free(ptr[1]) == 0);
   Mem_Dump();
printf("\n");
   ptr[2] = Mem_Alloc(32);
   ptr[3] = Mem_Alloc(8);
   Mem_Dump();
printf("\n");
   Mem_Free(ptr[2]);
   Mem_Free(ptr[3]);
   Mem_Dump();
   assert(Mem_Free(ptr[2]) == 0);
   assert(Mem_Free(ptr[3]) == 0);
  return 0;
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
  head->next = NULL;
  head->prev = NULL;
  close(fd);
  numCallsInit++;
  return 0;
  
}

void* Mem_Alloc(int size) {
  if(size % 8 != 0) {
    size += (8 - size % 8);
  }

  header_t *hPtr = NULL;
  int nSize = 0;
  void *memPtr = NULL;
  list_t *tmp = head;
  while(tmp) {
    if(tmp->size >= size + sizeof(header_t)) {
      memPtr = tmp;
      break;
    }
    tmp = tmp->next;
  }
  if(memPtr) {
    tmp = (void *) memPtr;
    list_t *tmpNext = tmp->next;
    nSize = tmp->size - size - sizeof(header_t);
    memPtr += (tmp->size - size);
    hPtr = (void *) memPtr - sizeof(header_t);
    hPtr->size = size;
    hPtr->magic = MAGIC;
    tmp->size = nSize;
    tmp->next = tmpNext;
  }
  else {
    return NULL;
  }
  return (void *) memPtr;
}

int Mem_Free(void* ptr) {
  if(!ptr)
    return -1;

  header_t *hptr = (void *) ptr - sizeof(header_t);

  if(hptr->magic != MAGIC)
    return -1;

  int freeSize = (hptr->size) + sizeof(header_t);
  list_t *tmp = head;
  head = (void *) hptr;
  tmp->prev = head;
  head->next = tmp;
  head->prev = NULL;
  head->size = freeSize;

  // Need to coalesce

  if(head + head->size == tmp) {
    head->size += tmp->size;
    if(tmp->next) {
      if(tmp + tmp->size == tmp->next) {
        head->size += tmp->next->size;
      }
    }
  }
  else if((tmp->next) && (tmp + tmp->size == tmp->next)) {
    tmp->size += tmp->next->size;
  }

  return 0;
  
}

int Mem_Available() {
  int numBytesAvailable = 0;
  list_t *tmp = head;
  while(tmp) {
    numBytesAvailable += (tmp->size - sizeof(header_t));
  }
  return numBytesAvailable;
}

void Mem_Dump() {
  list_t *tmp = head;
  while(tmp) {
    printf("Free Size: %d, Pointer: %p\n", tmp->size, tmp);
    tmp = tmp->next;
  }
}
