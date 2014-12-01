#include "mfs.h"
#include <stdio.h>
#include <stdlib.h>



int portNum = -1;
char *fileImage = NULL;



int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s <port number> <file-system-image>\n", argv[0]);
        exit(1);
    }


    return 0;
}