#define __USE_XOPEN_EXTENDED
#include "commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <iso646.h>
#include <errno.h>
#include <stdlib.h>

bool get_realpath(const char* path, char* buf)
{
    char* buffer = NULL;
    char *res = realpath(path, buffer);
    if (res) {
        strcpy(buf, res);
        return true;
    }
    else {
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

int ls_to_string(const char* path, char* destination)
{
    strcpy(destination, "");

    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    char buf[512];
    char buf2[STR_MAX];
    get_realpath(path, buf2);

    mydir = opendir(buf2);
    if (mydir == NULL)
    {
        printf("Unable to open folder %s.", buf2);
        return FORBIDDEN;
    }

    while((myfile = readdir(mydir)) != NULL)
    {
        // printf("bruh %s\n", path);
        sprintf(buf, "%s/%s", buf2, myfile->d_name);
        stat(buf, &mystat);
        if (strncmp(myfile->d_name, ".", 1)) // hidden file
        {
            sprintf(buf, "%s\n", myfile->d_name);
            strncat(destination, buf, STR_MAX);
        }
    }
    closedir(mydir);
    // printf("bruh %s\n", path);
    return 0;
}

int indexed_cat(const char* path, char* destination)
{
    FILE *f;
    int i;
    int retval = SUCCEXY;

    // abre o arquivo em leitura
    f = fopen ("dados.txt", "r") ;
    if (f == NULL)
    {
        if (errno == EACCES)
            retval =  FILE_DOES_NOT_EXIST;
        else
            retval =  FORBIDDEN;
    }
    if (retval != SUCCEXY)
    {
        printf("%s reading failed\n", path);
        return retval;
    }

    char buffer[STR_MAX + 10];
    char line[STR_MAX];
    strcpy(destination, "");


    // lê TODAS as linhas do arquivo
    i = 1 ;
    fgets (line, STR_MAX, f);
    while (not feof (f))
    {
        sprintf(buffer, "%d: %s", i, line);
        strcat(destination, buffer);
        fgets (line, STR_MAX, f) ;   // tenta ler a próxima linha
        i++ ;
    }

    // fecha o arquivo
    fclose (f) ;

    return retval;
}

// int main()
// {
//     ls(".");
//     return 0;
// }