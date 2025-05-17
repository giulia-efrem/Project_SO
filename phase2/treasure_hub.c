#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

typedef struct 
{
    int   treasureId;
    char  username[50];
    float latitude;
    float longitude;
    char  clue[100];
    int   value;
} Treasure;

static pid_t monitor_pid = 0;
static int   fds[2];      // parent->child
static int   result_fds[2];   // child->parent
static bool  stopping = false;

// new command
static volatile sig_atomic_t got_cmd = 0;

// start on SIGUSR1, stop on SIGUSR2
static void on_cmd(int sig)  
{ 
    got_cmd = 1; 
}
static void on_stop(int sig) 
{ 
    usleep(500000); 
    _exit(0); 
}

static int only_dirs(const struct dirent *e) 
{
    return e->d_type == DT_DIR
        && strcmp(e->d_name, ".")
        && strcmp(e->d_name, "..");
}

static void monitor_loop(void) 
{
    struct sigaction sa1 = { .sa_handler = on_cmd };
    sigaction(SIGUSR1, &sa1, NULL);

    struct sigaction sa2 = { .sa_handler = on_stop };
    sigaction(SIGUSR2, &sa2, NULL);

    close(fds[1]);  // child reads
    printf("[Monitor %d] is ready for commands…\n", getpid());

    while (1) 
    {
        pause();
        if (!got_cmd) continue;
        got_cmd = 0;

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
                perror("opendir");
                continue; 
            }

            struct dirent *e;
            while ((e = readdir(d))) 
            {
                if (!only_dirs(e)) continue;
                char binpath[512];
                snprintf(binpath, sizeof(binpath),"%s/%s/treasure.bin", base, e->d_name);
                
                FILE *tf = fopen(binpath, "rb");
                int cnt = 0;
                if (tf) 
                {
                    fseek(tf, 0, SEEK_END);
                    long sz = ftell(tf);

                    cnt = sz / sizeof(Treasure);
                    fclose(tf);
                }
                printf("%s: %d treasures\n", e->d_name, cnt);
            }
            closedir(d);
        }

        // list_treasures:<hunt>
        else if (strncmp(task, "list_treasures:",15) == 0) 
        {
            char *hunt = task + 15;
            char binpath[512];
            snprintf(binpath, sizeof(binpath),"../phase1/hunts/%s/treasure.bin", hunt);
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
                printf("  ID:%d user:%s loc:(%.4f,%.4f) value:%d clue:\"%s\"\n",t.treasureId, t.username,t.latitude, t.longitude,t.value, t.clue);
            }
            fclose(tf);
        }

        // view_treasure:<hunt>:<id>
        else if (strncmp(task, "view_treasure:",14) == 0) 
        {
            char *rest = task + 14;
            char *hunt = strtok(rest, ":");
            char *id_str = strtok(NULL, ":");

            if (!hunt || !id_str) continue;

            int tid = atoi(id_str);
            char binpath[512];
            snprintf(binpath, sizeof(binpath),"../phase1/hunts/%s/treasure.bin", hunt);
            
            FILE *tf = fopen(binpath, "rb");
            if (!tf) 
            {
                printf("Hunt '%s' has no treasure.bin\n", hunt);
                continue;
            }

            Treasure t;
            int found = 0;

            while (fread(&t, sizeof(Treasure), 1, tf) == 1) 
            {
                if (t.treasureId == tid) 
                {
                    printf("Treasure %d details:\n", t.treasureId);
                    printf("  User:      %s\n", t.username);
                    printf("  Location:  %.4f, %.4f\n",
                           t.latitude, t.longitude);
                    printf("  Value:     %d\n", t.value);
                    printf("  Clue:      %s\n", t.clue);
                    found = 1;
                    break;
                }
            }

            if (!found)
                printf("Treasure ID %d not found in hunt '%s'\n", tid, hunt);
            fclose(tf);
        }
    }
}

static void parent_sigchld(int sig) 
{
    int status;
    pid_t p = waitpid(monitor_pid, &status, WNOHANG);

    if (p == monitor_pid) 
    {
        printf("\n[Monitor %d] exited\n", monitor_pid);
        monitor_pid = 0;
        stopping = false;
    }
}

void start_monitor(void) 
{
    if (monitor_pid) 
    {
        printf("Monitor %d is already running\n", monitor_pid);
        return;
    }

    if (pipe(fds)    == -1) 
    { 
        perror("cmd pipe");
        exit(1); 
    }

    if (pipe(result_fds) == -1) 
    {
        perror("result pipe");
        exit(1); 
    }


    pid_t pid = fork();

    if (pid < 0) 
    { 
        perror("fork");
        return; 
    }

    if (pid == 0) 
    {
         
       close(fds[1]);       // only read commands
       close(result_fds[0]);    // only write results

       // redirecting stdout into the write end of result_fds
       if (dup2(result_fds[1], STDOUT_FILENO) == -1) 
       {
           perror("Err");
           exit(1);
       }
       close(result_fds[1]);

       monitor_loop();  // printf() in the monitor goes into the pipe
    }

    // parent
    close(fds[0]);
    close(result_fds[1]);
    monitor_pid = pid;
    stopping = false;

    printf("Started monitor %d\n", monitor_pid);
}

static void read_monitor_output() 
{
    char buf[512];
    fd_set rd;
    struct timeval tv;

    tv.tv_sec  = 0;
    tv.tv_usec = 200000;

    FD_ZERO(&rd);
    FD_SET(result_fds[0], &rd);

    // as long as select says there’s data, read & print
    while (select(result_fds[0]+1, &rd, NULL, NULL, &tv) > 0) 
    {
        ssize_t n = read(result_fds[0], buf, sizeof(buf)-1);
        if (n <= 0) break;
        buf[n] = '\0';
        fputs(buf, stdout);

        // reset timeout 
        tv.tv_sec  = 0;
        tv.tv_usec = 200000;
        FD_ZERO(&rd);
        FD_SET(result_fds[0], &rd);
    }
}

void list_hunts_cmd(void) 
{
    if (!monitor_pid || stopping)
    {
        puts("Error: monitor not available");
        return;
    }

    write(fds[1], "list_hunts", 11);
    kill(monitor_pid, SIGUSR1);

    read_monitor_output();

}

void list_treasures_cmd(void) 
{
    if (!monitor_pid || stopping) 
    {
        puts("the monitor not available");
        return;
    }

    char hunt[64];
    printf("Hunt name: ");
    fgets(hunt, sizeof(hunt), stdin);

    hunt[strcspn(hunt, "\n")] = 0;

    char msg[80];
    snprintf(msg, sizeof(msg), "list_treasures:%s", hunt);
    
    write(fds[1], msg, strlen(msg));
    kill(monitor_pid, SIGUSR1);

    read_monitor_output();

}

void view_treasure_cmd(void)
{
    if (!monitor_pid || stopping) 
    {
        puts("the monitor not available");
        return;
    }

    char hunt[64], id[16];

    printf("Hunt name: ");
    fgets(hunt, sizeof(hunt), stdin);
    hunt[strcspn(hunt, "\n")] = 0;

    printf("Treasure ID: ");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = 0;

    char msg[100];
    snprintf(msg, sizeof(msg), "view_treasure:%s:%s", hunt, id);
    write(fds[1], msg, strlen(msg));
    kill(monitor_pid, SIGUSR1);
    read_monitor_output();

}

void stop_monitor_cmd(void) 
{
    if (!monitor_pid || stopping)
    {
        puts("No monitor to stop");
        return;
    }

    stopping = true;
    kill(monitor_pid, SIGUSR2);
    printf("Stopping monitor %d…\n", monitor_pid);
}

int main(void) 
{
    // catch SIGCHLD
    struct sigaction sc = { .sa_handler = parent_sigchld };
    sigaction(SIGCHLD, &sc, NULL);

    char cmd[64];
    while (1) 
    {
        printf("--> ");
        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        cmd[strcspn(cmd, "\n")] = 0;

        if      (strcmp(cmd, "start_monitor")  == 0) start_monitor();
        else if (strcmp(cmd, "list_hunts")     == 0) list_hunts_cmd();
        else if (strcmp(cmd, "list_treasures") == 0) list_treasures_cmd();
        else if (strcmp(cmd, "view_treasure")  == 0) view_treasure_cmd();
        else if (strcmp(cmd, "stop_monitor")   == 0) stop_monitor_cmd();
        else if (strcmp(cmd, "exit")           == 0) 
        {
            if (monitor_pid || stopping)
                puts("Error: monitor still running");
            else
                break;
        }
        else
            printf("unknown: '%s'\n", cmd);
    }
    return 0;
}
