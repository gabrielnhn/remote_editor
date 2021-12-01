#ifndef __COMMANDS__
#define __COMMANDS__

#include <stdbool.h>

#define STR_MAX 100

// no operation / local operation
#define NOP 0 


#define CD 1
#define LCD 2
#define LS 3
#define LLS 4
#define VER 5
#define LINHA 6
#define LINHAS 7
#define EDIT 8
#define COMPILAR 9

bool get_realpath(char* path, char* buf);

bool cd(char* path, char* current_dir);

int ls(char* path);

#endif