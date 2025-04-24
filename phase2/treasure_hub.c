#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void start_monitor()   
{ 
    printf("not yet\n"); 
}

void list_hunts()      
{ 
    printf("not yet\n"); 
}

void list_treasures()  
{ 
    printf("not yet\n"); 
}

void view_treasure()   
{ 
    printf("not yet\n"); 
}

void stop_monitor()    
{ 
    printf("not yetr\n"); 
}

int main() 
{
  char cmd[64];
  
  while (1) 
  {
    printf("--> ");
    if (!fgets(cmd, sizeof(cmd), stdin)) break;

    cmd[strcspn(cmd, "\n")] = 0;

    if (strcmp(cmd, "start_monitor")==0)   
        start_monitor();
    
    else if (strcmp(cmd, "list_hunts")==0) 
        list_hunts();
    
    else if (strcmp(cmd, "list_treasures")==0) 
        list_treasures();
    
    else if (strcmp(cmd, "view_treasure")==0) 
        view_treasure();
    
    else if(strcmp(cmd, "stop_monitor")==0)
        stop_monitor();
    
    else if (strcmp(cmd, "exit")==0)       
        break;
    
    else printf("unknown: '%s'\n", cmd);
  }
  return 0;
}
