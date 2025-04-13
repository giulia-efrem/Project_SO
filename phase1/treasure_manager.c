#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

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

    printf("Treasure added to hunt '%s'\n", huntName);
    return 0;
}

int listHunt(const char *huntName)
{
    char huntDir[256];
    snprintf(huntDir, sizeof(huntDir), "hunts/%s", huntName);

    struct stat st;
    if (stat(huntDir, &st) != 0 || !S_ISDIR(st.st_mode)) 
    {
        printf("Hunt '%s' doesn't exist\n", huntName);
        return -1;
    }

    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/treasure.bin", huntDir);
    FILE *f = fopen(treasurePath, "rb");

    if (!f) 
    {
        printf("No treasure.bin found in hunt '%s'.\n", huntName);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    rewind(f);

    long sizeKB = (fileSize + 1023) / 1024;
    struct tm *timeinfo = localtime(&st.st_mtime);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

    printf("%s treasure.bin %ldKB %s\n", huntName, sizeKB, timeStr);

    int numRecords = fileSize / sizeof(Treasure);

    Treasure t;
    for (int i = 0; i < numRecords; i++) 
    {
        if (fread(&t, sizeof(Treasure), 1, f) != 1) 
        {
            break;  
        }
        printf("treasure %d: %s %.2f %.2f %d %s\n", t.treasureId, t.username, t.latitude, t.longitude, t.value, t.clue);
    }
    fclose(f);

    char generalLogPath[256], huntLogPath[256], logEntry[256];
    snprintf(generalLogPath, sizeof(generalLogPath), "logs/general.log");
    snprintf(huntLogPath, sizeof(huntLogPath), "logs/%s.log", huntName);
    snprintf(logEntry, sizeof(logEntry), "Listed treasures for hunt '%s'", huntName);
    logMessage(generalLogPath, logEntry);
    logMessage(huntLogPath, logEntry);

    return 0;
}


int viewTreasure(const char *huntName, int treasureId)
{
    char huntDir[256];
    snprintf(huntDir, sizeof(huntDir), "hunts/%s", huntName);

    struct stat st;
    if (stat(huntDir, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        printf("Hunt '%s' doesn't exist\n", huntName);
        return -1;
    }

    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/treasure.bin", huntDir);

    FILE *f = fopen(treasurePath, "rb");
    if (!f)
    {
        perror("Error opening treasure.bin");
        return -1;
    }

    Treasure t;
    int found = 0;
    while (fread(&t, sizeof(Treasure), 1, f) == 1)
    {
        if (t.treasureId == treasureId)
        {
            printf("Treasure ID: %d\n", t.treasureId);
            printf("Username: %s\n", t.username);
            printf("Latitude: %f\n", t.latitude);
            printf("Longitude: %f\n", t.longitude);
            printf("Clue: %s\n", t.clue);
            printf("Value: %d\n", t.value);

            found = 1;
            break;
        }
    }
    fclose(f);

    if (!found)
    {
        printf("Treasure ID %d not found in hunt '%s'.\n", treasureId, huntName);
        return -1;
    }

    char generalLogPath[256], huntLogPath[256], logEntry[256];
    snprintf(generalLogPath, sizeof(generalLogPath), "logs/general.log");
    snprintf(huntLogPath, sizeof(huntLogPath), "logs/%s.log", huntName);
    snprintf(logEntry, sizeof(logEntry),"Treasure ID %d viewed from hunt '%s'.",treasureId, huntName);
    logMessage(generalLogPath, logEntry);
    logMessage(huntLogPath, logEntry);

    return 0;
}

int removeTreasure(const char *huntName, int treasureId)
{
    char huntDir[256];
    snprintf(huntDir, sizeof(huntDir), "hunts/%s", huntName);

    struct stat st;
    if (stat(huntDir, &st) != 0 || !S_ISDIR(st.st_mode)) 
    {
        printf("Hunt '%s' doesn't exist\n", huntName);
        return -1;
    }

    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/treasure.bin", huntDir);

    FILE *inFile = fopen(treasurePath, "rb");
    if (!inFile) 
    {
        printf("Can't open the treasure file for reading\n");
        return -1;
    }

    char tempPath[256];
    snprintf(tempPath, sizeof(tempPath), "%s/treasure_temp.bin", huntDir);
    FILE *outFile = fopen(tempPath, "wb");

    if (!outFile) 
    {
        fclose(inFile);
        perror("Error creating a temporary file");
        return -1;
    }

    Treasure t;
    int found = 0;

    while (fread(&t, sizeof(Treasure), 1, inFile) == 1) 
    {
        if (t.treasureId == treasureId) 
        {
            found = 1;
        }

        else 
        {
            fwrite(&t, sizeof(Treasure), 1, outFile);
        }
    }

    fclose(inFile);
    fclose(outFile);

    if (!found) 
    {
        printf("Treasure ID %d was not found in hunt '%s'\n", treasureId, huntName);
        remove(tempPath);
        return -1;
    }

    if (remove(treasurePath) != 0) 
    {
        perror("Error removing the original treasure file");
        return -1;
    }

    if (rename(tempPath, treasurePath) != 0) 
    {
        perror("Error renaming the temporary file to the treasure file");
        return -1;
    }

    char generalLogPath[256];
    snprintf(generalLogPath, sizeof(generalLogPath), "logs/general.log");

    char huntLogPath[256];
    snprintf(huntLogPath, sizeof(huntLogPath), "logs/%s.log", huntName);

    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "Treasure ID %d was removed from hunt '%s'", treasureId, huntName);

    logMessage(generalLogPath, logEntry);
    logMessage(huntLogPath, logEntry);

    printf("Treasure ID %d was removed from hunt '%s'\n", treasureId, huntName);
    return 0;
}

int removeHunt(const char *huntName)
{
    char huntDir[256];
    snprintf(huntDir, sizeof(huntDir), "hunts/%s", huntName);

    struct stat st;
    if (stat(huntDir, &st) != 0 || !S_ISDIR(st.st_mode)) 
    {
        printf("Hunt '%s' doesn't exist\n", huntName);
        return -1;
    }

    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/treasure.bin", huntDir);
    if (remove(treasurePath) != 0 && errno != ENOENT) 
    {
        perror("Error removing the treasure.bin");
        return -1;
    }

    if (rmdir(huntDir) != 0)
    {
        perror("Error removing the hunt directory");
        return -1;
    }

    char huntLogPath[256];
    snprintf(huntLogPath, sizeof(huntLogPath), "logs/%s.log", huntName);

    if (remove(huntLogPath) != 0 && errno != ENOENT) 
    {
        perror("Error removing the hunt log file");
    }

    char generalLogPath[256];
    snprintf(generalLogPath, sizeof(generalLogPath), "logs/general.log");

    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "Hunt '%s' removed", huntName);

    logMessage(generalLogPath, logEntry);

    printf("Hunt '%s' was removed \n", huntName);
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
        if (argc < 3) 
        {
            printf("Use: %s --list <huntName>\n", argv[0]);
            return 1;
        }
        return listHunt(argv[2]);
    }

    else if (strcmp(argv[1], "--view") == 0) 
    {
        if (argc < 4)
        {
            printf("Use: %s --view <huntName> <treasureId>\n", argv[0]);
            return 1;
        }

        const char *huntName = argv[2];
        int treasureId = atoi(argv[3]);

        return viewTreasure(huntName, treasureId);
    }

    else if (strcmp(argv[1], "--removeTreasure") == 0)
    {
        if (argc < 4)
        {
            printf("Use: %s --remove <huntName> <treasureId>\n", argv[0]);
            return 1;
        }

        const char *huntName = argv[2];
        int treasureId = atoi(argv[3]);
        return removeTreasure(huntName, treasureId);
    }

    else if (strcmp(argv[1], "--removeHunt") == 0)
    {
        if (argc < 3)
        {
            printf("Use: %s --remove_hunt <huntName>\n", argv[0]);
            return 1;
        }
        return removeHunt(argv[2]);
    }

    else 
    {
        printf("Not the right command\n");
    }

    return 0;
}
