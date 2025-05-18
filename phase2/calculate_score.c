// calculate_score.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct 
{
    int  treasureId;
    char username[50];
    float latitude;
    float longitude;
    char clue[100];
    int  value;
} Treasure;

typedef struct UserScore 
{
    char username[50];
    int  score;
    struct UserScore *next;
} UserScore;

int main(int argc, char **argv) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s <huntName>\n", argv[0]);
        return 1;
    }
    char *hunt = argv[1];
    char path[256];
    snprintf(path, sizeof(path),"../phase1/hunts/%s/treasure.bin", hunt);

    FILE *f = fopen(path, "rb");
    if (!f) 
    {
        fprintf(stderr, "Cannot open %s: %s\n",
                path, strerror(errno));
        return 1;
    }

    UserScore *head = NULL;
    Treasure t;
    while (fread(&t, sizeof t, 1, f) == 1) 
    {
        UserScore *u = head, *prev = NULL;
        while (u && strcmp(u->username, t.username))
            prev = u, u = u->next;

        if (!u) 
        {
            u = malloc(sizeof *u);
            strcpy(u->username, t.username);
            u->score = 0;
            u->next  = NULL;
            if (prev) prev->next = u;
            else      head       = u;
        }
        u->score += t.value;
    }
    fclose(f);

    // print
    for (UserScore *u = head; u; u = u->next)
        printf("%s: %d\n", u->username, u->score);

    while (head) 
    {
        UserScore *tmp = head->next;
        free(head);
        head = tmp;
    }
    return 0;
}
