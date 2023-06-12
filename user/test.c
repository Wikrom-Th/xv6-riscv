#include "kernel/types.h"

#include "kernel/stat.h"

#include "user/user.h"


int main(void) 
{   
    char* name;
    uint64 val;
    
    val = getname();
    name = (char*) val;

    //this causes a usertrap...
    printf("The name is: %s\n", name);
    exit(0);
} 