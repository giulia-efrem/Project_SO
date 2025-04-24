#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <unistd.h>     

void monitor_loop() 
{
    printf("[Monitor %d] ready for commands…\n", getpid());
    
    while (1) 
    {
        pause();
    }
}

void start_monitor() 
{
    
    if (monitor_pid) 
    {
        printf("[Monitor %d] already running\n", monitor_pid);
        return;
    }

    pid_t pid = fork();

    if (pid < 0) 
    {
        perror("fork");
        return;
    }

    if (pid == 0) 
    {
        monitor_loop();
        exit(0);
    }
    monitor_pid = pid;
    printf("Started monitor %d\n", monitor_pid);
}

void list_hunts()      
{ 
    
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
