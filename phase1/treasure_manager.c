#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        printf("Not enough arguments");
        return 1;
    }

    if (strcmp(argv[1], "--add") == 0) 
    {
    }
    else if (strcmp(argv[1], "--add_treasure") == 0) 
    {
    }
    else if (strcmp(argv[1], "--list") == 0) 
    {
    }
    else if (strcmp(argv[1], "--view") == 0) 
    {
    }
    else 
    {
        printf("Not the right command\n");
    }

    return 0;
}
