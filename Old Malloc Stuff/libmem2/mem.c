#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"


// Header for each block of memory giving information about it
typedef struct header{
    struct header* next;
    int blocksize;

}Header;

Header* allocatedHead = NULL;


int Mem_Init(int size)
{
    int pagesize;
    int roundingsize;
    int fd;
    int sizeToRequest;
    void* ptr;
    static int alreadyAllocated = 0;

    if(alreadyAllocated != 0)
    {
        fprintf(stderr,"Error memory already initialized\n");
        return -1;
    }

    if(size <= 0)
    {
        fprintf(stderr,"Error size must be positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    //round up to the next page size
    roundingsize = size % pagesize;
    roundingsize = (pagesize - roundingsize) % pagesize;

    //get the final amount to allocate
    sizeToRequest = size + roundingsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if(-1 == fd)
    {
        fprintf(stderr,"error cannot open /dev/zero\n");
        return -1;
    }

    ptr = mmap(NULL, sizeToRequest, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (ptr == MAP_FAILED)
    {
        fprintf(stderr,"error can't allocate space\n");
        alreadyAllocated = 0;
        return -1;
    }

    alreadyAllocated = 1;

    // To start with, there is only one big, free block
    allocatedHead = (Header*)ptr;
    allocatedHead->next = NULL;

    //set pointer to the front of the free space not includint the header
    allocatedHead->blocksize = sizeToRequest - (int)sizeof(Header);
    
    return 0;
}

void* Mem_Alloc(int size)
{
    if(size <= 0)
    {
        return NULL;
    }

    //while(size%4 != 0) size++;
    //adjust size requested to be multiple of 4
    if((size % 4) != 0)
    {
        size += (size % 4);
    }

    // Get an iterator for the list of free memory
    Header* itr = allocatedHead;

    while(itr != NULL)
    {
        int status = ((unsigned int)itr->blocksize) & 1; 

        if(status == 0)
        {
            //room for a new block
            if(itr->blocksize >= size + sizeof(Header))
            {
                //save refrence to next header
                Header *temp = itr->next;

                //create new header for memory
                Header *newHeader;

                //point to the new headers location
                newHeader = ((Header *)(itr + (itr->blocksize)/sizeof(Header) - size/sizeof(Header))); 

                //set the new header to be allocated by incrementing size
                newHeader->blocksize = size + 1;

                //new headers next is the next node in the list which is temp
                newHeader->next = temp;

                //link newheader into the lsit
                itr->next = newHeader;

                //adjust the size
                itr->blocksize -= size + sizeof(Header);
                
                // Return the correct adress
                return newHeader + 1;
            }
            //perfect fit
            if(itr->blocksize == size)
            {
                //set status bit
                itr->blocksize++;

                return itr+1;
            }
        }

        //looked through whole list
        if(itr->next == NULL)
        {
            return NULL;
        }

        itr = itr->next;
    }

    return NULL;
}

int Mem_Free(void* ptr)
{
    if( ptr == NULL )
    {
        return -1;
    }

    //find the header
    Header* pHeader = (Header *)(ptr) - 1;

    //check to see if the status is free
    if( (pHeader->blocksize & 1) == 0 )
    {
        return -1;
    }

    //set status bit to 0
    pHeader->blocksize--;

    //If the next chunk is empty
    if( pHeader->next != NULL && ((pHeader->next->blocksize) & 1) == 0 )
    {
        //set size to the new total size
        pHeader->blocksize += pHeader->next->blocksize + sizeof(Header);

        //set next to nexts next
        pHeader->next = pHeader->next->next;
    } 

    //If the previous chunk is empty
    Header* itr = allocatedHead;
    while(itr != NULL && itr->next != NULL)
    {
        //if next is what we are removing
        if( itr->next == pHeader )
        {
            //the the previous is empty
            if( ((itr->blocksize) & 1) == 0 )
            {
                //set the new size
                itr->blocksize += itr->next->blocksize + sizeof(Header);

                //set the next node
                itr->next = itr->next->next;
            }
        }

        itr = itr->next;
    }
    
    return 0;
}

int Mem_Available()
{
    return 0;
}

void Mem_Dump()
{
    int count;
    int freeSize;
    int usedSize;
    int totalSize;
    int size;
    int t_size;

    char* t_start = NULL;
    char* start = NULL;
    char* end = NULL;
    char status[5];

    Header* curr = NULL;

    freeSize = 0;
    usedSize = 0;
    totalSize = 0;

    curr = allocatedHead;

    count = 1;

    fprintf(stdout,"No.\tstatus\tstart\t\tend\t\tsize\tt_size\tt_start\n\n");

    while(NULL != curr)
    {
        t_start = (char*)curr;
        start = t_start + (int)sizeof(Header);
        size = curr->blocksize;
        strcpy(status,"Free");

        //LSB = 1 => busy block
        if(size & 1)
        {
            strcpy(status, "Busy");

            //ignore status in busy block
            size--;
            t_size = size + (int)sizeof(Header);
            usedSize = usedSize + t_size;
        }
        else
        {
            t_size = size + (int)sizeof(Header);
            freeSize = freeSize + t_size;
        }

        end = start + size;
        fprintf(stdout,"%d\t%s\t0x%08lx\t0x%08lx\t%d\t%d\t0x%08lx\n",count ,status ,(unsigned long int)start, 
            (unsigned long int)end, size, t_size, (unsigned long int)t_start);

        totalSize = totalSize + t_size;
        curr = curr->next;
        count++;

    }

    fprintf(stdout,"\n~~~~~~~~~~~~~~\n");
    fprintf(stdout,"Total allocated = %d\n",usedSize);
    fprintf(stdout,"Total free = %d\n",freeSize);
    fprintf(stdout,"Total size = %d\n",usedSize+freeSize);
    fprintf(stdout,"~~~~~~~~~~~~~~\n");
    fflush(stdout);

    return;
}
