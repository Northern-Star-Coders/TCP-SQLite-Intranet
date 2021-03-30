#ifndef __TCPSQLITE_H
#define __TCPSQLITE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

// struttura dati che fa da base per costruzione del messaggio

// define nomi righe messaggio client
#define PROT_HEADER "/0.1"
#define PROT_TABLE "Table"
#define PROT_COMMAND "Command"
#define PROT_ID "Id"
#define PROT_COLUMNS "Columns"
#define PROT_VALUES "Values"
#define PROT_STATUSCODE "StatusCode"
#define PROT_DETAILS "Details"
#define PROT_DATA "Data"

// define comandi della riga command
#define CMD_INIT "INIT"
#define CMD_CREATE "CREATE"
#define CMD_DROP "DROP"
#define CMD_INSERT "INSERT"
#define CMD_UPDATE "UPDATE"
#define CMD_SELECT "SELECT"
#define CMD_DELETE "DELETE"

#define CALLBACK_INIT_DELETE "sqlite_sequence"

#define LOOPBACK "127.0.0.1"
#define SEPARATOR ", "

#define MAX_MSG 1024 * 3
#define MAX_COLS 50
#define MAX_STR 1024
#define MAX_CONN 50

char callback_retval[MAX_MSG];

typedef struct
{
    char *table;
    char *command;
    int id;
    char **columns;
    char **values;
} request_fields;

typedef struct
{
    int status_code;
    const char *details;
    char *data;
} response_fields;

request_fields init_request_fields();
request_fields compile_request_fields(char *, char *, int, char **, char **);
void print_request_fields(request_fields);
void pack_request(char *, request_fields);
void unpack_request(request_fields *, char *);

response_fields init_response_fields();
response_fields compile_response_fields(int, const char *, char *);
void print_response_fields(response_fields);
void pack_response(char *, response_fields);
void unpack_response(response_fields *, char *);
char **split(const char *, const char *);

request_fields init_request_fields()
{
    return compile_request_fields(NULL, NULL, 0, NULL, NULL);
}

request_fields compile_request_fields(char *table, char *command, int id, char **columns, char **values)
{
    request_fields fields;
    fields.table = table;
    fields.command = command;
    fields.id = id;
    fields.columns = columns;
    fields.values = values;
    return fields;
}

void print_request_fields(request_fields fields)
{
    printf("Request fields{\n");
    printf("\tTable: %s\n", fields.table);
    printf("\tCommand: %s\n", fields.command);
    printf("\tId: %d\n", fields.id);

    int i = 0;
    printf("\tColumns: ");
    if (fields.columns == NULL)
    {
        printf("(null)");
    }
    else
    {
        while (*(fields.columns + i))
        {
            printf("%s", *(fields.columns + i));
            if (*(fields.columns + i + 1))
                printf(SEPARATOR);
            i++;
        }
    }
    printf("\n");

    i = 0;
    printf("\tValues: ");
    if (fields.values == NULL)
    {
        printf("(null)");
    }
    else
    {
        while (*(fields.values + i))
        {
            printf("%s", *(fields.values + i));
            if (*(fields.values + i + 1))
                printf(SEPARATOR);
            i++;
        }
    }
    printf("\n");
    
    printf("}\n");
}

void pack_request(char *request, request_fields fields)
{
    *request = '\0';

    int i = 0;
    char cols[MAX_STR];
    if (fields.columns == NULL)
    {
        strcpy(cols, "(null)");
    }
    else
    {
        while (*(fields.columns + i))
        {
            strcat(cols, *(fields.columns + i));
            if (*(fields.columns + i + 1))
                strcat(cols, SEPARATOR);
            i++;
        }
    }

    i = 0;
    char vals[MAX_STR];
    if (fields.values == NULL)
    {
        strcpy(vals, "(null)");
    }
    else
    {
        while (*(fields.values + i))
        {
            strcat(vals, *(fields.values + i));
            if (*(fields.values + i + 1))
                strcat(vals, SEPARATOR);
            i++;
        }
    }

    char buffer[MAX_MSG];

    sprintf(buffer, "%s\n%s: %s\n%s: %s\n%s: %d\n%s: %s\n%s: %s\n", PROT_HEADER, PROT_TABLE, fields.table, PROT_COMMAND, fields.command, PROT_ID, fields.id, PROT_COLUMNS, cols, PROT_VALUES, vals);
    strcat(request, buffer);
}

void unpack_request(request_fields *fields, char *request)
{
    char *table = strdup(strstr(request, PROT_TABLE) + strlen(PROT_TABLE) + 2);
    *(strstr(table, "\n")) = '\0';
    fields->table = table;

    char *command = strdup(strstr(request, PROT_COMMAND) + strlen(PROT_COMMAND) + 2);
    *(strstr(command, "\n")) = '\0';
    fields->command = command;

    char *id = strdup(strstr(request, PROT_ID) + strlen(PROT_ID) + 2);
    *(strstr(id, "\n")) = '\0';
    fields->id = atoi(id);

    char *columns = strdup(strstr(request, PROT_COLUMNS) + strlen(PROT_COLUMNS) + 2);
    *(strstr(columns, "\n")) = '\0';
    fields->columns = split(columns, SEPARATOR);

    char *values = strdup(strstr(request, PROT_VALUES) + strlen(PROT_VALUES) + 2);
    *(strstr(values, "\n")) = '\0';
    fields->values = split(values, SEPARATOR);
}

/* RESPONSE */

response_fields init_response_fields()
{
    return compile_response_fields(0, NULL, NULL);
}

response_fields compile_response_fields(int status_code, const char *details, char *data)
{
    response_fields fields;
    fields.status_code = status_code;
    fields.details = details;
    fields.data = data;
    return fields;
}

void print_response_fields(response_fields fields)
{
    printf("Response fields{\n");
    printf("\tStatusCode: %d\n", fields.status_code);
    printf("\tDetails: %s\n", fields.details);
    printf("\tData: %s\n", fields.data);
    printf("}\n");
}

void pack_response(char *response, response_fields fields)
{
    *response = '\0';
    char buffer[MAX_MSG];
    snprintf(buffer, MAX_MSG, "%s\n%s: %d\n%s: %s\n%s: %s\n", PROT_HEADER, PROT_STATUSCODE, fields.status_code, PROT_DETAILS, fields.details, PROT_DATA, fields.data);
    strcat(response, buffer);
}

void unpack_response(response_fields *fields, char *response)
{
    char *status_code = strdup(strstr(response, PROT_STATUSCODE) + strlen(PROT_STATUSCODE) + 2);
    *(strstr(status_code, "\n")) = '\0';
    fields->status_code = atoi(status_code);

    char *details = strdup(strstr(response, PROT_DETAILS) + strlen(PROT_DETAILS) + 2);
    *(strstr(details, "\n")) = '\0';
    fields->details = details;

    char *data = strdup(strstr(response, PROT_DATA) + strlen(PROT_DATA) + 2);
    *(strstr(data, "\n")) = '\0';
    fields->data = data;
}

char **split(const char *s, const char *seps)
{
    char *temp[MAX_COLS + 1];
    char *s1 = strdup(s);
    char *token = strtok(s1, seps);

    int i = 0;
    for (i = 0; token != NULL; i++)
    {
        temp[i] = strdup(token);
        token = strtok(NULL, seps);
    }
    temp[i] = NULL;
    free(s1);

    char **ret = (char **)calloc(i, sizeof(char *));
    for (i = 0; *(ret + i) = temp[i]; i++){;}
    return ret;
}

int init_callback(void *flag, int numCols, char *rowValues[], char *colsName[])
{
    flag = 0;
    char buffer[MAX_STR];

    for (int i = 0; i < numCols; i++)
    {
        sprintf(buffer, "%s, ", rowValues[i] ? rowValues[i] : "NULL");
        strcat(callback_retval, buffer);
    }

    return 0;
}

void init_callback_style(){
    char *temp = strdup(strstr(callback_retval, CALLBACK_INIT_DELETE) + strlen(CALLBACK_INIT_DELETE) + 2);
    *(temp + strlen(temp) - 2) = '\0';
    callback_retval[0] = '\0';
    strcat(callback_retval, temp);
}

#endif //__TCPSQLITE_H