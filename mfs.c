#include "mfs.h"

// typedef struct __MFS_Stat_t {
//     int type;   // MFS_DIRECTORY or MFS_REGULAR
//     int size;   // bytes
//     // note: no permissions, access times, etc.
// } MFS_Stat_t;

// typedef struct __MFS_DirEnt_t {
//     char name[60];  // up to 60 bytes of name in directory (including \0)
//     int  inum;      // inode number of entry (-1 means entry not used)
// } MFS_DirEnt_t;


int MFS_Init(char *hostname, int port)
{

	return -1;
}


int MFS_Lookup(int pinum, char *name)
{

	return -1;
}


int MFS_Stat(int inum, MFS_Stat_t *m)
{

	return -1;
}


int MFS_Write(int inum, char *buffer, int block)
{

	return -1;
}


int MFS_Read(int inum, char *buffer, int block)
{

	return -1;
}


int MFS_Creat(int pinum, int type, char *name)
{

	return -1;
}


int MFS_Unlink(int pinum, char *name)
{

	return -1;
}


int MFS_Shutdown()
{

	return -1;
}

