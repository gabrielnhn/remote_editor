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

bool get_realpath(const char* path, char* buf);

int cd(char* path, char* current_dir);

int ls_to_string(const char* path, char* destination);

int ls(char* path);

int indexed_cat(const char* path, char* destination);

int check_filename(const char* path);

int get_line(const char* path, int line_index, char* destination);

int get_lines(const char* path, int line1, int line2, char* destination);

#endif