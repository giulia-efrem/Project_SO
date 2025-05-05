#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <unistd.h>     
#include <dirent.h>

pid_t monitor_pid = 0;

int fds[2]; 

int only_dirs(const struct dirent *e) {
    return (e->d_type == DT_DIR
        && strcmp(e->d_name, ".")
        && strcmp(e->d_name, ".."));
}

void monitor_loop() 
{
    printf("[Monitor %d] is ready for commands…\n", getpid());

    while (1) 
    {
        char task[64];

        ssize_t n = read(fds[0], task, sizeof(task)-1);

        if (n > 0) {

            task[n] = '\0';

            if (strcmp(task, "list_hunts")==0) {
                const char *path = "../phase1/hunts";
                DIR *d = opendir(path);
                if (!d) {
                    perror("opendir");
                    exit(0);
                }

                struct dirent *entry;
                while ((entry = readdir(d)) != NULL) {
                    if (entry->d_name[0] == '.' &&
                        (entry->d_name[1] == '\0' ||
                        (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
                        continue;

                    printf("%s\n", entry->d_name);
                }

                closedir(d);
            }

        }


    }
}

void start_monitor() 
{
    if (pipe(fds) == -1) 
    {
        perror("pipe");
        exit(1);
    }
    
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
        close(fds[1]); 
        monitor_loop();
        exit(0);
    }

    close(fds[0]); // we make parent write-only
    monitor_pid = pid;
    printf("Started monitor %d\n", monitor_pid);
}

void list_hunts()      
{ 
    const char *task = "list_hunts";
    write(fds[1], task, strlen(task));
    
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
