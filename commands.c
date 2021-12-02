#include "commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <iso646.h>
#include <errno.h>

int get_cwd(char* buf) {

    char cwd[STR_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strcpy(buf, cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }
    return 0;
}

bool get_realpath(char* path, char* buf)
{
    char *res = realpath(path, buf);
    if (res) { // or: if (res != NULL)
        return true;
    } else {
        return false;
    }
    return 0;
}

int cd(char* path, char* current_dir)
{
    int retval;

    DIR *mydir;
    mydir = opendir(path);

    if ((mydir != NULL) and get_realpath(path, current_dir) and (chdir(current_dir) == 0))
        retval = SUCCEXY;
    else 
    {
        if (errno == EACCES)
            retval = FORBIDDEN;
        else if ((errno == ENOTDIR) or (errno == ENOENT))
            retval = DIR_DOES_NOT_EXIST;
    }
    closedir(mydir);

    return retval;
}

int ls(char* path)
{
    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    char buf[512];
    mydir = opendir(path);
    while((myfile = readdir(mydir)) != NULL)
    {
        sprintf(buf, "%s/%s", path, myfile->d_name);
        stat(buf, &mystat);

        if (strncmp(myfile->d_name, ".", 1)) // hidden file
            printf("%s\n", myfile->d_name);
    }
    closedir(mydir);
    return 0;
}

int ls_to_string(char* path, char* destination)
{
    strcpy(destination, "");

    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    char buf[512];
    mydir = opendir(path);
    while((myfile = readdir(mydir)) != NULL)
    {
        sprintf(buf, "%s/%s", path, myfile->d_name);
        stat(buf, &mystat);

        if (strncmp(myfile->d_name, ".", 1)) // hidden file
        {
            sprintf(buf, "%s\n", myfile->d_name);
            strncat(destination, buf, STR_MAX);
        }
    }
    closedir(mydir);
    return 0;
}

// int main()
// {
//     ls(".");
//     return 0;
// }