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



// //Global Variables
// int portNum = -1;
// char *fileImage = NULL;


// int main(int argc, char *argv[])
// {

//     if (argc != 3) 
//     {
//         fprintf(stderr, "Usage: %s <port number> <file-system-image>\n", argv[0]);
//         exit(1);
//     }

//     //Get and check the first arg
//     portNum = atoi(argv[1]);
//     if(portNum < 0)
//     {
//         fprintf(stderr, "Error: Port Number must be positive.\n", argv[0]);
//         exit(1);
//     }

//     //Get and check the second arg
//     fileImage = argv[2];


//     printf("First Arg: %d \n Second Arg: %s\n", portNum, fileImage );


//     return 0;
// } 
