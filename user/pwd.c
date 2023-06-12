//Thamchareonkit Wikrom 72075605
//Print full path of the current working directory

#include "kernel/types.h"
#include "kernel/fs.h"
#include "user/user.h"

char* mystrcat(char* str1, char* str2) {
    char* combined = malloc(strlen(str1) + strlen(str2) + 1);

    strcpy(combined, str1);
    strcpy(combined+strlen(str1) , str2);

    return combined;
}

int diropen(char* path) {
    int fd;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "Cannot open: .\n");
        exit(1);
    }

    return fd;
}

char* traverse_back_dir(char* path, ushort prev_inum, char* outpath) {
    ushort curr_inum = 0;
    int fd = diropen(path);

    char* next_path = mystrcat("../", path);

    struct dirent dir;
    int cc;

    while ((cc = read(fd, &dir, 16)) == 16) {
        if (dir.inum == 0) continue;

        if (!strcmp(dir.name, ".")) {

            if(prev_inum == 0) {
                //handling first recursion
                close(fd);
                return traverse_back_dir(next_path, dir.inum, outpath);
            }

            else {
                //so that the most nested directory doesn't have a trailing slash
                if(strcmp(outpath, "")) {
                    outpath = mystrcat("/", outpath);
                }
                curr_inum = dir.inum;
            }
        }

        if (prev_inum == dir.inum) {
            close(fd);
            outpath = mystrcat(dir.name, outpath);

            //final recursion
            if (curr_inum == 1) {
                if (prev_inum == 1) return "/";

                outpath = mystrcat("/", outpath);
                return outpath;
            }
            return traverse_back_dir(next_path, curr_inum, outpath);
        }
    }

    //have to do this otherwise the compiler will complain
    fprintf(2, "Error: program reaches unreachable code.\n");
    return outpath;
}

int main()
{
    char* working_dir;

    //always call with this argument at the starting recursion     
    working_dir = traverse_back_dir(".", 0, "");

    printf("%s\n", working_dir);
    exit(0);
}