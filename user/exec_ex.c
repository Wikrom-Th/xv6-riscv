#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "usage: %s cmd args\n", argv[0]);
        exit(1);
    }

    if (fork() == 0)
    {
        exec(argv[1], argv + 1);
        fprintf(2, "exec error: %s\n", argv[1]);
        exit(1);
    }
    else
    {
        wait(0);
    }

    exit(0);
}
