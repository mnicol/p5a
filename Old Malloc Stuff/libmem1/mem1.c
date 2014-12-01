#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

//for the only 16 byte workload


/* this structure serves as the header for each block */
typedef struct block_hd{
  /* The blocks are maintained as a linked list */
  /* The blocks are ordered in the increasing order of addresses */
  struct block_hd* next;

  /* size of the block is always a multiple of 4 */
  /* ie, last two bits are always zero - can be used to store other information*/
  /* LSB = 0 => free block */
  /* LSB = 1 => allocated/busy block */

  /* For free block, block size = size_status */
  /* For an allocated block, block size = size_status - 1 */

  /* The size of the block stored here is not the real size of the block */
  /* the size stored here = (size of block) - (size of header) */
  int size_status;

}block_header;

/* Global variable - This will always point to the first block */
/* ie, the block with the lowest address */
block_header* list_head = NULL;

int Mem_Init(int size)
{
  // if(size <= 0){return -1;}

  // //////////instructor code
  // //// open the /dev/zero device
  // //int fd = open("/dev/zero", O_RDWR);

  // //// size (in bytes) needs to be evenly divisible by the page size
  // //void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  // //if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }

  // //// close the device (don't worry, mapping should be unaffected)
  // //  close(fd);



  // //round up to next page size

  // //getpagezize()

  // //use mmap
  //   return 0;
  int pagesize;
  int padsize;
  int fd;
  int alloc_size;
  void* space_ptr;
  static int allocated_once = 0;
  
  if(0 != allocated_once)
  {
    fprintf(stderr,"Error:mem.c: Mem_Init has allocated space during a previous call\n");
    return -1;
  }
  if(size <= 0)
  {
    fprintf(stderr,"Error:mem.c: Requested block size is not positive\n");
    return -1;
  }

  /* Get the pagesize */
  pagesize = getpagesize();

  /* Calculate padsize as the padding required to round up sizeOfRegio to a multiple of pagesize */
  padsize = size % pagesize;
  padsize = (pagesize - padsize) % pagesize;

  alloc_size = size + padsize;

  /* Using mmap to allocate memory */
  fd = open("/dev/zero", O_RDWR);
  if(-1 == fd)
  {
    fprintf(stderr,"Error:mem.c: Cannot open /dev/zero\n");
    return -1;
  }
  space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (MAP_FAILED == space_ptr)
  {
    fprintf(stderr,"Error:mem.c: mmap cannot allocate space\n");
    allocated_once = 0;
    return -1;
  }
  
  allocated_once = 1;
  
  /* To begin with, there is only one big, free block */
  list_head = (block_header*)space_ptr;
  list_head->next = NULL;
  /* Remember that the 'size' stored in block size excludes the space for the header */
  list_head->size_status = alloc_size - (int)sizeof(block_header);
  
  return 0;
}

void* Mem_Alloc(int size)
{
  if(size <= 0) return NULL;
  while(size%4 != 0) size++;

  block_header* iterator = list_head;
  while(iterator != NULL)
  {
    int status = ((unsigned int)iterator->size_status) & 0x1; 
    if(status == 0)
    {
      /*if block has room for new block*/
      if(iterator->size_status >= size + sizeof(block_header))
      {
        /*hold on to next header*/
        block_header *temp = iterator->next;
        /*create new header...*/
        block_header *new_hd;
        /*...at iterator + iterator_size - size*/
        new_hd = ((block_header *)(iterator + (iterator->size_status)/sizeof(block_header) - size/sizeof(block_header))); 
        /*set new status to allocated*/
        new_hd->size_status = size+1;
        /*set next of new to old prev's next (temp*)*/
        new_hd->next = temp;
        /*set iterator's next to new*/
        iterator->next = new_hd;
        /*set iterators new (smaller) size*/
        iterator->size_status -= size + sizeof(block_header);
        
        return new_hd + 1;
      }
      /*if block fits perfectly*/
      if(iterator->size_status == size)
      {
        /*set iterator status bit*/
        iterator->size_status++;

        return iterator+1;
      }
    }
    /*searched entire list*/
    if(iterator->next == NULL) return NULL;
    
    iterator = iterator->next;
  }

  return NULL;
}

int Mem_Free(void* ptr)
{
    if( ptr == NULL ) return -1;
  
  /*find header*/
  block_header* ptr_hd = (block_header *)(ptr) - 1;
  
  /*ptr header is status free*/
  if( (ptr_hd->size_status & 0x1) == 0 ) return -1;
  
  /*set status to 0*/
  ptr_hd->size_status--;

  /*IF next is empty*/
  if( ptr_hd->next != NULL && ((ptr_hd->next->size_status) & 0x1) == 0 ){
    /*set ptr's size to the new total size (orig size + next's size + 1 block header)*/
    ptr_hd->size_status += ptr_hd->next->size_status + sizeof(block_header);
    /*set ptr's next to my next's next*/
    ptr_hd->next = ptr_hd->next->next;
  } 

  /*IF prev is empty*/
  block_header* iterator = list_head;
  while(iterator != NULL && iterator->next != NULL)
  {
    /*if next is the ptr we are removing*/
    if( iterator->next == ptr_hd )
      /*the previous is found empty*/
      if( ((iterator->size_status) & 0x1) == 0 )
      {
        /*set size to size of prev's size + removed size + size of block header*/
        iterator->size_status += iterator->next->size_status + sizeof(block_header);
        /*set prev's next to next's next*/
        iterator->next = iterator->next->next;
      }
    iterator = iterator->next;
  }
  return 0;
}

int Mem_Available()
{

    return 0;
}

void Mem_Dump()
{
  int counter;
  block_header* current = NULL;
  char* t_Begin = NULL;
  char* Begin = NULL;
  int Size;
  int t_Size;
  char* End = NULL;
  int free_size;
  int busy_size;
  int total_size;
  char status[5];

  free_size = 0;
  busy_size = 0;
  total_size = 0;
  current = list_head;
  counter = 1;
  fprintf(stdout,"************************************Block list***********************************\n");
  fprintf(stdout,"No.\tStatus\tBegin\t\tEnd\t\tSize\tt_Size\tt_Begin\n");
  fprintf(stdout,"---------------------------------------------------------------------------------\n");
  while(NULL != current)
  {
    t_Begin = (char*)current;
    Begin = t_Begin + (int)sizeof(block_header);
    Size = current->size_status;
    strcpy(status,"Free");
    if(Size & 1) /*LSB = 1 => busy block*/
    {
      strcpy(status,"Busy");
      Size = Size - 1; /*Minus one for ignoring status in busy block*/
      t_Size = Size + (int)sizeof(block_header);
      busy_size = busy_size + t_Size;
    }
    else
    {
      t_Size = Size + (int)sizeof(block_header);
      free_size = free_size + t_Size;
    }
    End = Begin + Size;
    fprintf(stdout,"%d\t%s\t0x%08lx\t0x%08lx\t%d\t%d\t0x%08lx\n",counter,status,(unsigned long int)Begin,(unsigned long int)End,Size,t_Size,(unsigned long int)t_Begin);
    total_size = total_size + t_Size;
    current = current->next;
    counter = counter + 1;
  }
  fprintf(stdout,"---------------------------------------------------------------------------------\n");
  fprintf(stdout,"*********************************************************************************\n");

  fprintf(stdout,"Total busy size = %d\n",busy_size);
  fprintf(stdout,"Total free size = %d\n",free_size);
  fprintf(stdout,"Total size = %d\n",busy_size+free_size);
  fprintf(stdout,"*********************************************************************************\n");
  fflush(stdout);
  return;
}
