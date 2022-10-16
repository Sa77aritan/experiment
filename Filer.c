//
// Created by marco on 16.10.22.
//

#include <unistd.h>
#include <string.h>
#include <bits/types/FILE.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

struct chatmessages {
    char *user;
    char *date;
    char *message;
};

int checkIfFileExists(char filename[]);
int createFile(char filename[]);
int deleteFile(char filename[]);
void writeToFile(char filename[], char name[], char date[], char message[]);
char *getLineValue(char filename[], int column);
char *readFile(char filename[]);
struct chatmessages *getLineValueStruct(char filename[]);
void printLooper(struct chatmessages *list);

int main()
{
    if (checkIfFileExists("test.csv") == 0)
    {
        createFile("test");
    }
    writeToFile("test.csv", "Franz", "18:05 16.10.2022", "Hey, are you free?");
    writeToFile("test.csv", "Peter3", "18:07 16.10.2022", "Yes I am");

    struct chatmessages *list = NULL;
    list = getLineValueStruct("test.csv");

    printLooper(list);

    free(list);
}


/**
 * Überprüft ob die Datei filename im aktuellen Verzeichnis existiert.
 * @param filename
 * @return 1 falls ja, 0 falls nein.
 */
int checkIfFileExists(char filename[])
{
    if (access(filename, F_OK) == 0)
    {
        return 1;
    }
    return 0;
}

/**
 * Erstellt im aktuellen Verzeichnis eine Datei mit dem Namen filename.csv.
 * @param filename
 * @return 0 = kein Fehler, 1 = Fehler
 */
int createFile(char filename[])
{
    if (checkIfFileExists(filename) == 0)
    {
        char filenameCSV[strlen(filename) + 5];
        // Initialisieren damit strcat funktioniert.
        filenameCSV[0] = 0;
        strcat(filenameCSV, filename);
        strcat(filenameCSV, ".csv");

        FILE *fp;
        // File wird erstellt
        fp = fopen(filenameCSV, "w+");
        fclose(fp);
        return 0;
    }
    return 1;
}

/**
 * Löscht die Datei filename.
 * @param filename
 * @return 0 = kein Fehler, 1 = Fehler
 */
int deleteFile(char filename[])
{
    if (remove(filename) == 0) {
        return 0;
    }
    return 1;
}

//todo: use a struct
void writeToFile(char filename[], char name[], char date[], char message[])
{
    FILE *fp;
    fp = fopen(filename, "a+");
    fprintf(fp, "%s;%s;%s;\n", name, date, message);
}

void readFromFile()
{

}


// todo: Const char* oder char*
/**
 * Lädt ganze Datei in den Heap.
 * Memory muss am ende wieder freigegeben werden. (free *char pointer)
 * @param filename
 * @return char pointer
 */
char *readFile(char filename[])
{
    char* fileAsAString = NULL;

    FILE *file = fopen(filename, "r");
    size_t n = 0;
    int c;

    if (file == NULL)
    {
        perror("Datei konnte nicht geöffnet werden\n");
        return NULL;
    }

    long f_size = ftell(file);
    fileAsAString = malloc(f_size);

    while ((c = fgetc(file)) != EOF)
    {
        fileAsAString[n++] = (char)c;
    }

    // terminating string with '\0'
    fileAsAString[n] = '\0';

    return fileAsAString;
}

char *getLineValues(char *fileAsAString, int column)
{
    char* tok;
    for (tok = strtok(fileAsAString, ";");
         tok && *tok;
         tok = strtok(NULL, ";\n"))
    {
        if (!--column)
            return tok;
    }
    return NULL;
}

char *getLineValue(char filename[], int column)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Unable to open the file.");
        exit(1);
    }
    char line[200];

    while (fgets(line, sizeof(line), fp))
    {
        char *token;
        token = strtok(line, ";");

        int counter = 0;
        while (token != NULL)
        {
            printf("Position: %i Token: %s\n\n", counter, token);
            token = strtok(NULL, ";\n");

            counter++;



        }
        printf("\n");
    }
}

/**
 * Achtung, nicht länger als 200!!!
 * @param filename
 * @param column
 * @return
 */
struct chatmessages *getLineValueStruct(char filename[])
{
    // todo: make dynamic
    struct chatmessages *pMessageList = malloc(sizeof(struct chatmessages));

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Unable to open the file.");
        exit(1);
    }
    char line[200];

    int rowCounter = 0;

    while (fgets(line, sizeof(line), fp))
    {
        char *token;
        token = strtok(line, ";");

        int columnCounter = 0;
        while (token != NULL)
        {
            //printf("Position: %i Token: %s\n\n", columnCounter, token);
            token = strtok(NULL, ";\n");

            if (columnCounter == 0)
            {
                // initialize memory for struct
                pMessageList[rowCounter].user = strdup(token);
            } else if (columnCounter == 1)
            {
                pMessageList[rowCounter].date = strdup(token);
            } else if (columnCounter == 2)
            {
                pMessageList[rowCounter].message = strdup(token);
            }

            columnCounter++;
        }
        printf("\n");

        rowCounter++;
    }
}

void printer(struct chatmessages *list)
{
    printf("Contents of chatmessage:\n");
    printf("name: %s", list->user);
    printf("date: %s", list->date);
    printf("message: %s", list->message);
}

void printLooper(struct chatmessages *list)
{
    for (int i = 0; i < (sizeof(list)/sizeof(list[0])); i++)
    {
        printer(&list[i]);
    }
}