#ifndef __COMMANDS__
#define __COMMANDS__

#include <stdbool.h>

#define STR_MAX 100
#define A_LOT 100000

// RETURN VALUES
#define SUCCEXY 0
// Succexy, song by Metric
// https://open.spotify.com/track/2RCW6UGqMit6POan7k5h6X

#define FORBIDDEN 1
#define DIR_DOES_NOT_EXIST 2
#define FILE_DOES_NOT_EXIST 3
#define LINE_DOES_NOT_EXIST 4


// no operation / local operation
#define NOP -1

bool get_realpath(char* path, char* buf);

int cd(char* path, char* current_dir);

int ls_to_string(char* path, char* destination);

int ls(char* path);

int indexed_cat(char* path, char* destination);

int check_filename(char* path);

int get_line(char* path, int line_index, char* destination);

int get_lines(char* path, int line1, int line2, char* destination);

int edit(char* path, int index, char* new_line);

int compile(char* path, char* destination);

#endif
