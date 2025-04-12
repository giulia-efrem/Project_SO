#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct 
{
    int treasureId;

    char username[50];
    float latitude;
    float longitude;
    char clue[100];
    int value;
} Treasure;

void logMessage(const char *filename, const char *message) 
{
    FILE *f = fopen(filename, "a");
    if (f) 
    {
        fprintf(f, "%s\n", message);
        fclose(f);
    }
}

int addHunt(const char *huntName) 
{

    if (mkdir("hunts", 0777) == -1 && errno != EEXIST) 
    {
        perror("Error creating hunts directory");
        return -1;
    }


    char huntDir[256];
    snprintf(huntDir, sizeof(huntDir), "hunts/%s", huntName);
    if (mkdir(huntDir, 0777) == -1) 
    {
        perror("Error creating hunt directory");
        return -1;
    }


    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/treasure.bin", huntDir);
    FILE *treasureFile = fopen(treasurePath, "wb");
    if (!treasureFile) 
    {
        perror("Error creating treasure.bin");
        return -1;
    }
    fclose(treasureFile);


    if (mkdir("logs", 0777) == -1 && errno != EEXIST) 
    {
        perror("Error creating logs directory");
        return -1;
    }


    char generalLogPath[256];
    snprintf(generalLogPath, sizeof(generalLogPath), "logs/general.log");
    
    char huntLogPath[256];
    snprintf(huntLogPath, sizeof(huntLogPath), "logs/%s.log", huntName);
    

    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "Hunt '%s' created.", huntName);
    logMessage(generalLogPath, logEntry);
    logMessage(huntLogPath, logEntry);

    printf("Hunt '%s' was added\n", huntName);
    return 0;
}

int addTreasure(const char *huntName) 
{

    char huntDir[256];
    snprintf(huntDir, sizeof(huntDir), "hunts/%s", huntName);


    struct stat st;
    if (stat(huntDir, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        printf("Hunt '%s' doesn't exist, create it\n", huntName);
        return -1;
    }


    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/treasure.bin", huntDir);
    FILE *f = fopen(treasurePath, "ab");
    if (!f) 
    {
        perror("Error opening treasure.bin");
        return -1;
    }


    Treasure t;
    printf("Treasure ID: ");
    scanf("%d", &t.treasureId);

    printf("Username: ");
    scanf("%s", t.username);

    printf("Latitude: ");
    scanf("%f", &t.latitude);

    printf("Longitude: ");
    scanf("%f", &t.longitude);

    printf("Clue: ");

    getchar(); 
    scanf("%[^\n]", t.clue);

    printf(" Value: ");
    scanf("%d", &t.value);


    fwrite(&t, sizeof(Treasure), 1, f);
    fclose(f);

    char generalLogPath[256];
    snprintf(generalLogPath, sizeof(generalLogPath), "logs/general.log");

    char huntLogPath[256];
    snprintf(huntLogPath, sizeof(huntLogPath), "logs/%s.log", huntName);

    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry),
             "Treasure ID %d added to hunt '%s' by user '%s'.",
             t.treasureId, huntName, t.username);

    logMessage(generalLogPath, logEntry);
    logMessage(huntLogPath, logEntry);

    printf("Treasure added to hunt '%s'.\n", huntName);
    return 0;
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        printf("Not enough arguments");
        return 1;
    }

    if (strcmp(argv[1], "--add") == 0) 
    {
        if (argc < 3) 
        {
            printf("Use: %s --add <huntName>\n", argv[0]);
            return 1;
        }

        const char *huntName = argv[2];
        if (addHunt(huntName) != 0) 
        {
            printf("Failed to add hunt '%s'\n", huntName);
            return 1;
        }
    }
    else if (strcmp(argv[1], "--add_treasure") == 0) 
    {
        if (argc < 3) 
        {
            printf("Use: %s --add_treasure <huntName>\n", argv[0]);
            return 1;
        }

        if (addTreasure(argv[2]) != 0) 
        {
            printf("Failed to add treasure to hunt '%s'.\n", argv[2]);
            return 1;
        }
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
