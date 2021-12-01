#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

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
}

// int main()
// {
//     ls(".");
//     return 0;
// }