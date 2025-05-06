#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <unistd.h>     
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct 
{
    int   treasureId;
    char  username[50];
    float latitude;
    float longitude;
    char  clue[100];
    int   value;
} Treasure;

pid_t monitor_pid = 0;
int   fds[2]; 

int only_dirs(const struct dirent *e) 
{
    return e->d_type == DT_DIR
        && strcmp(e->d_name, ".")
        && strcmp(e->d_name, "..");
}

void on_stop(int sig) 
{
    usleep(500000);  // simulate delay
    _exit(0);
}

void monitor_loop() 
{
    struct sigaction sa = {0};
    sa.sa_handler = on_stop;
    sigaction(SIGUSR1, &sa, NULL);

    printf("[Monitor %d] is ready for commands…\n", getpid());
    
    while (1) 
    {
        char task[128];
        ssize_t n = read(fds[0], task, sizeof(task)-1);
        if (n <= 0) continue;
        task[n] = '\0';

        // list_hunts
        if (strcmp(task, "list_hunts") == 0) 
        {
            const char *base = "../phase1/hunts";
            DIR *d = opendir(base);
            if (!d) 
            { 
                perror("opendir"); exit(1); 
            }

            struct dirent *entry;
            while ((entry = readdir(d))) 
            {
                if (!only_dirs(entry)) continue;

                char binpath[512];
                snprintf(binpath, sizeof(binpath),
                         "%s/%s/treasure.bin", base, entry->d_name);

                FILE *tf = fopen(binpath, "rb");

                int cnt = 0;
                if (tf) 
                {
                    if (fseek(tf, 0, SEEK_END)==0) 
                    {
                        long sz = ftell(tf);
                        cnt = sz / sizeof(Treasure);
                    }
                    fclose(tf);
                }
                printf("%s: %d treasures\n", entry->d_name, cnt);
            }
            closedir(d);
        }

        // list_treasures:<hunt>
        else if (strncmp(task, "list_treasures:",15) == 0)
        {
            char *hunt = task + 15;
            char binpath[512];
            snprintf(binpath, sizeof(binpath), "../phase1/hunts/%s/treasure.bin", hunt);

            FILE *tf = fopen(binpath, "rb");

            if (!tf) 
            {
                printf("Hunt '%s' has no treasure.bin\n", hunt);
                continue;
            }

            printf("Treasures in '%s':\n", hunt);

            Treasure t;
            while (fread(&t, sizeof(Treasure), 1, tf) == 1) 
            {
                printf("  ID: %d user: %s (%.4f,%.4f) value: %d clue:\"%s\"\n", t.treasureId, t.username,t.latitude, t.longitude,t.value, t.clue);
            }
            fclose(tf);
        }
    }
}

void start_monitor() 
{
    if (pipe(fds) == -1) 
    { 
        perror("pipe"); exit(1); 
    }

    if (monitor_pid) 
    {
        printf("[Monitor %d] is already running\n", monitor_pid);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) 
    { 
        perror("fork"); return; 
    }

    if (pid == 0) 
    {
        close(fds[1]);
        monitor_loop();
    }

    close(fds[0]);
    monitor_pid = pid;
    printf("Started monitor %d\n", monitor_pid);
}

void list_hunts() 
{
    write(fds[1], "list_hunts", strlen("list_hunts"));
    usleep(200000);
}

void list_treasures()  
{ 
    char hunt[64];
    printf("Hunt name: ");
    if (!fgets(hunt, sizeof(hunt), stdin)) return;
    hunt[strcspn(hunt, "\n")] = 0;

    char task[80];
    snprintf(task, sizeof(task), "list_treasures:%s", hunt);
    write(fds[1], task, strlen(task));
    usleep(200000);
}

void view_treasure()   
{ 
    printf("not yet\n"); 
}

void stop_monitor() 
{
    if (!monitor_pid) 
    {
        printf("No monitor running\n");
        return;
    }

    kill(monitor_pid, SIGUSR1);
    waitpid(monitor_pid, NULL, 0);
    
    printf("Monitor %d stopped\n", monitor_pid);
    
    monitor_pid = 0;
}

int main() 
{
    char cmd[64];

    while (1) 
    {
        printf("--> ");
        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        cmd[strcspn(cmd, "\n")] = 0;

        if      (strcmp(cmd, "start_monitor") == 0) start_monitor();
        else if (strcmp(cmd, "list_hunts")   == 0) list_hunts();
        else if (strcmp(cmd, "list_treasures")== 0) list_treasures();
        else if (strcmp(cmd, "view_treasure")== 0) view_treasure();
        else if (strcmp(cmd, "stop_monitor")== 0) stop_monitor();
        else if (strcmp(cmd, "exit")         == 0) break;
        else    printf("unknown: '%s'\n", cmd);
    }
    return 0;
}
