#ifndef __COMMANDS__
#define __COMMANDS__

#include <stdbool.h>

#define STR_MAX 100
#define A_LOT 100000

// RETURN VALUES
#define SUCCEXY 0
#define FORBIDDEN 1
#define DIR_DOES_NOT_EXIST 2
#define FILE_DOES_NOT_EXIST 3
#define LINE_DOES_NOT_EXIST 4


// no operation / local operation
#define NOP -1
#define ONE_NIGHT_STAND 1
#define STREAM 2

bool get_realpath(char* path, char* buf);

int cd(char* path, char* current_dir);

int ls(char* path);

#endif