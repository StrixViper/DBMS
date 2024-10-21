#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TABLES 100
#define MAX_ROWS 100
#define MAX_COLUMNS 4
#define MAX_NAME_LENGTH 50
#define MAX_VALUE_LENGTH 50
#define MAX_COMMAND_LENGTH 1024
#define MAX_FILENAME_LENGTH 256


typedef enum {
    INT,
    STRING
}DataType; //enum to hold the data type of the columns ---> for future upgrades!

typedef struct {
    char columnName[MAX_NAME_LENGTH];
} Column;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    Column columns[MAX_COLUMNS];
    int columnCount;
} CreateQuery;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    char values[MAX_COLUMNS][MAX_VALUE_LENGTH];
    int valueCount;
} InsertQuery;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    char columnName[MAX_NAME_LENGTH];
    char newValue[MAX_VALUE_LENGTH];
    char conditionColumn1[MAX_NAME_LENGTH];
    char conditionValue1[MAX_VALUE_LENGTH];
    char conditionColumn2[MAX_NAME_LENGTH];
    char conditionValue2[MAX_VALUE_LENGTH];
    int hasAndCondition;
    int hasOrCondition;
} UpdateQuery;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    char condition1[MAX_NAME_LENGTH];
    char condition2[MAX_NAME_LENGTH];
    int hasAnd;
    int hasOr;
} SelectQuery;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    char conditionColumn[MAX_NAME_LENGTH];
    char conditionValue[MAX_VALUE_LENGTH];
    char conditionColumn2[MAX_NAME_LENGTH];
    char conditionValue2[MAX_VALUE_LENGTH];
    int hasAnd;
} DeleteQuery;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
} DropQuery;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    char columnNames[MAX_COLUMNS][MAX_NAME_LENGTH]; // Array to hold column names
    int columnCount; // Number of columns
} ShowQuery;

typedef struct {
    char indexName[MAX_NAME_LENGTH];
    char tableName[MAX_NAME_LENGTH];
    char columnNames[MAX_COLUMNS][MAX_NAME_LENGTH];
    int columnCount;
} IndexQuery;

// Example list to hold created indexes
IndexQuery indexes[MAX_TABLES]; // Assuming max indexes equal to max tables for simplicity
int indexCount = 0;

typedef struct {
    char values[MAX_COLUMNS][MAX_VALUE_LENGTH];
} Row;

typedef struct {
    char tableName[MAX_NAME_LENGTH];
    Column columns[MAX_COLUMNS];
    Row rows[MAX_ROWS];
    int rowCount;
    int columnCount;
    int TableID;
} Table;

Table database[MAX_TABLES];
int tableCount = 0;

typedef enum {
    CREATE,
    INSERT,
    SELECT,
    UPDATE,
    DROP,
    DELETE,
    UNKNOWN
} SqlCommands;


//########################----FUNCTION DEFINITION------################################

void ClearConsole(); //function to clear the console context

void sleep_in_seconds(int seconds); //function to give delay to the program for any OS

void DisplayMenu(); //function to display the menu for the user and activate the user's choice

void FromChoiceToCommand(int num); //convert the user choice to the relevant function

void logCommand(const char *command); //function to log command into a file

void uploadCommandsFromFile(const char *filename); //function to upload command from file line by line and proccess it

SqlCommands getCommandType(const char *command); //fuction to compare the first words of a command to identify the type

void createTable(CreateQuery *createQuery);

void insertRow(InsertQuery *insertQuery);

int checkCondition(const Table *table, const Row *row, const char *columnName, const char *value);

void selectRows(SelectQuery *selectQuery);

void updateRow(UpdateQuery *updateQuery);

void dropTable(DropQuery *dropQuery);

void DeleteRow(DeleteQuery *deleteQuery);

void createIndex(IndexQuery *indexQuery); //function to create a index(not perfectly done yet)

void dropIndex(const char *indexName); //function to drop a index we created

void processCommand(const char *command); //function to take a command and process it using the previous functions

void showTable(ShowQuery *showQuery); //function to display a table in a form of a table

void ListAllTables(); //function to list all the tables the user create on the DB

void WriteCommand(); //function to take commands from the user

int GetColumnCount(char *tableName); //function to get the Column Count by a specific table name

int GetRowCount(char *tableName); //function to get the Row Count by a specific table name

//##########################################################################################

int main() {

    while(1) DisplayMenu();

    return 0;

}

void ClearConsole() {
    #if defined(_WIN32)
        system("cls"); // For Windows
    #else
        system("clear"); // For Linux and macOS
    #endif
}

void sleep_in_seconds(int seconds) {
    #ifdef _WIN32
        Sleep(seconds * 1000);  // Sleep expects milliseconds on Windows
    #elif __linux__ || __APPLE__
        sleep(seconds);         // Sleep expects seconds on Linux/macOS
    #else
        printf("Unsupported OS\n");
    #endif
}

void DisplayMenu() {

    int choice;
    printf("\n");
    printf("|**************************************|\n");
    printf("|      DATABASE MANAGEMENT SYSTEM      |\n");
    printf("|**************************************|\n");
    printf("|             Menu Options             |\n");
    printf("|--------------------------------------|\n");
    printf("|  1. Load Commands                    |\n"); //the user should write the file path
    printf("|  2. Print All Tables                 |\n"); //the user should write the file path to print the table
    printf("|  3. Write Commands                   |\n"); //function that input the command should run and then Process command should activate
    printf("|  0. EXIT                             |\n");
    printf("|--------------------------------------|\n");
    printf("   Please enter your choice: ");
    scanf("%d", &choice);
    printf("|--------------------------------------|\n");

    FromChoiceToCommand(choice);
}

void FromChoiceToCommand(int num) {
    switch(num) {
    case 1: {
        char filename[MAX_FILENAME_LENGTH];
        printf("Insert the file name: ");
        getchar(); // Consume the newline
        fgets(filename, MAX_FILENAME_LENGTH, stdin);
        filename[strcspn(filename, "\n")] = 0; // Remove newline character
        uploadCommandsFromFile(filename);
        break;
    }
    case 2:
        ListAllTables();
        break;
    case 3:
        WriteCommand(); // Assuming WriteCommand is already defined
        break;
    case 0:
        printf("Exiting the program.\n");
        sleep_in_seconds(2);
        break;
    default:
        printf("Invalid choice. Please try again.\n");
        break;
    }
}

void logCommand(const char *command) {
    FILE *file = fopen("commands.log", "a");
    if (file == NULL) {
        printf("Error opening log file!\n");
        return;
    }
    fprintf(file, "%s\n", command);
    fclose(file);
}

void uploadCommandsFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the file: %s\n", filename);
        return;
    }

    char command[256];
    while (fgets(command, sizeof(command), file)) {
        // Remove newline character from the command
        command[strcspn(command, "\n")] = '\0';
        processCommand(command);
    }

    fclose(file);
}

SqlCommands getCommandType(const char *command) {
    if (strncmp(command, "CREATE TABLE", 12) == 0) return CREATE;
    if (strncmp(command, "INSERT INTO", 11) == 0) return INSERT;
    if (strncmp(command, "SELECT", 6) == 0) return SELECT;
    if (strncmp(command, "SET", 3) == 0) return UPDATE;
    if (strncmp(command, "DROP TABLE", 10) == 0) return DROP;
    if (strncmp(command, "DELETE FROM", 11) == 0) return DELETE;
    return UNKNOWN;
}

void createTable(CreateQuery *createQuery) {
    if (tableCount >= MAX_TABLES) {
        printf("No space left, talk with the manager for a bigger package!!\n");
        return;
    }

    Table newTable;
    strcpy(newTable.tableName, createQuery->tableName);
    newTable.TableID = tableCount + 1;
    newTable.rowCount = 0;
    newTable.columnCount = createQuery->columnCount;

    for (int i = 0; i < createQuery->columnCount; i++) {
        strcpy(newTable.columns[i].columnName, createQuery->columns[i].columnName);
    }

    database[tableCount++] = newTable;
    printf("Table %s created with %d columns.\n", createQuery->tableName, createQuery->columnCount);
}

void insertRow(InsertQuery *insertQuery) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(database[i].tableName, insertQuery->tableName) == 0) {
            if (database[i].rowCount >= MAX_ROWS) {
                printf("Table %s is full.\n", insertQuery->tableName);
                return;
            }

            for (int j = 0; j < insertQuery->valueCount; j++) {
                strcpy(database[i].rows[database[i].rowCount].values[j], insertQuery->values[j]);
            }

            database[i].rowCount++;
            printf("Row inserted into %s.\n", insertQuery->tableName);
            return;
        }
    }
    printf("Table %s not found.\n", insertQuery->tableName);
}

int checkCondition(const Table *table, const Row *row, const char *columnName, const char *value) {
    for (int k = 0; k < table->columnCount; k++) {
        if (strcmp(table->columns[k].columnName, columnName) == 0 &&
            strcmp(row->values[k], value) == 0) {
            return 1; // Condition met
        }
    }
    return 0; // Condition not met
}

void selectRows(SelectQuery *selectQuery) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(database[i].tableName, selectQuery->tableName) == 0) {
            printf("Selected rows from %s:\n", selectQuery->tableName);
            for (int j = 0; j < database[i].rowCount; j++) {
                int condition1Met = 1; // Default to true if no conditions
                int condition2Met = 1; // Default to true if no second condition

                if (selectQuery->condition1[0] != '\0') {
                    char condColumn1[MAX_NAME_LENGTH], condValue1[MAX_VALUE_LENGTH];
                    sscanf(selectQuery->condition1, "%[^=]=%s", condColumn1, condValue1);
                    condition1Met = checkCondition(&database[i], &database[i].rows[j], condColumn1, condValue1);
                }

                if (selectQuery->hasAnd || selectQuery->hasOr) {
                    if (selectQuery->condition2[0] != '\0') {
                        char condColumn2[MAX_NAME_LENGTH], condValue2[MAX_VALUE_LENGTH];
                        sscanf(selectQuery->condition2, "%[^=]=%s", condColumn2, condValue2);
                        condition2Met = checkCondition(&database[i], &database[i].rows[j], condColumn2, condValue2);
                    }
                }

                // Print row if conditions are met or if no conditions are specified
                if ((selectQuery->hasAnd && condition1Met && condition2Met) ||
                    (selectQuery->hasOr && (condition1Met || condition2Met)) ||
                    (!selectQuery->hasAnd && !selectQuery->hasOr && condition1Met) ||
                    (selectQuery->condition1[0] == '\0' && selectQuery->condition2[0] == '\0')) { // Print if no conditions
                    printf("Row %d: ", j + 1);
                    for (int k = 0; k < database[i].columnCount; k++) {
                        printf("%s: %s ", database[i].columns[k].columnName, database[i].rows[j].values[k]);
                    }
                    printf("\n");
                }
            }
            return;
        }
    }
    printf("Table %s not found.\n", selectQuery->tableName);
}

void updateRow(UpdateQuery *updateQuery) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(database[i].tableName, updateQuery->tableName) == 0) {
            for (int j = 0; j < database[i].rowCount; j++) {
                int condition1Met = checkCondition(&database[i], &database[i].rows[j], updateQuery->conditionColumn1, updateQuery->conditionValue1);
                int condition2Met = 1; // Default to true if no second condition
                if (updateQuery->hasAndCondition || updateQuery->hasOrCondition) {
                    condition2Met = 0; // Default to false if second condition needs checking
                    if (strcmp(updateQuery->conditionColumn2, "") != 0) {
                        condition2Met = checkCondition(&database[i], &database[i].rows[j], updateQuery->conditionColumn2, updateQuery->conditionValue2);
                    }
                }

                // Update if conditions are met
                if ((updateQuery->hasAndCondition && condition1Met && condition2Met) ||
                    (updateQuery->hasOrCondition && (condition1Met || condition2Met)) ||
                    (!updateQuery->hasAndCondition && !updateQuery->hasOrCondition && condition1Met)) {
                    for (int k = 0; k < database[i].columnCount; k++) {
                        if (strcmp(database[i].columns[k].columnName, updateQuery->columnName) == 0) {
                            strcpy(database[i].rows[j].values[k], updateQuery->newValue);
                            printf("Updated %s to %s in table %s.\n", updateQuery->columnName, updateQuery->newValue, updateQuery->tableName);
                        }
                    }
                }
            }
            return;
        }
    }
    printf("Table %s not found.\n", updateQuery->tableName);
}

void dropTable(DropQuery *dropQuery) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(database[i].tableName, dropQuery->tableName) == 0) {
            // Shift all tables down to fill the gap
            for (int j = i; j < tableCount - 1; j++) {
                database[j] = database[j + 1];
            }
            tableCount--;
            printf("Table %s dropped.\n", dropQuery->tableName);
            return;
        }
    }
    printf("Table %s not found.\n", dropQuery->tableName);
}

void DeleteRow(DeleteQuery *deleteQuery) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(database[i].tableName, deleteQuery->tableName) == 0) {
            int rowsDeleted = 0;
            for (int j = 0; j < database[i].rowCount; j++) {
                int condition1Met = checkCondition(&database[i], &database[i].rows[j], deleteQuery->conditionColumn, deleteQuery->conditionValue);
                int condition2Met = 1; // Default to true if no second condition

                if (deleteQuery->hasAnd) {
                    condition2Met = checkCondition(&database[i], &database[i].rows[j], deleteQuery->conditionColumn2, deleteQuery->conditionValue2);
                }

                // Delete if conditions are met
                if ((deleteQuery->hasAnd && condition1Met && condition2Met) ||
                    (!deleteQuery->hasAnd && condition1Met)) {
                    // Shift rows up to fill the gap
                    for (int k = j; k < database[i].rowCount - 1; k++) {
                        database[i].rows[k] = database[i].rows[k + 1];
                    }
                    database[i].rowCount--;
                    rowsDeleted++;
                    j--; // Stay on the same index after deletion
                }
            }
            printf("Deleted %d row(s) from table %s.\n", rowsDeleted, deleteQuery->tableName);
            return;
        }
    }
    printf("Table %s not found.\n", deleteQuery->tableName);
}

void createIndex(IndexQuery *indexQuery) {
    if (indexCount >= MAX_TABLES) {
        printf("No space left for more indexes!\n");
        return;
    }
    // Store the index information
    indexes[indexCount++] = *indexQuery;
    printf("Index %s created on table %s for columns: ", indexQuery->indexName, indexQuery->tableName);
    for (int i = 0; i < indexQuery->columnCount; i++) {
        printf("%s ", indexQuery->columnNames[i]);
    }
    printf("\n");
}

void dropIndex(const char *indexName) {
    for (int i = 0; i < indexCount; i++) {
        if (strcmp(indexes[i].indexName, indexName) == 0) {
            // Shift indexes down to fill the gap
            for (int j = i; j < indexCount - 1; j++) {
                indexes[j] = indexes[j + 1];
            }
            indexCount--;
            printf("Index %s dropped.\n", indexName);
            return;
        }
    }
    printf("Index %s not found.\n", indexName);
}

void processCommand(const char *command) {
    printf("Processing command: %s\n", command);

    if (strlen(command) == 0) {
        printf("Command EMPTY, skipping.\n");
        return;
    }

    SqlCommands commandType = getCommandType(command);

    // Log the command and save it for later
    logCommand(command);

    if (commandType == CREATE) {
        CreateQuery createQuery;
        sscanf(command, "CREATE TABLE %s", createQuery.tableName);
        createQuery.columnCount = 0;

        char *token = strtok(command + strlen("CREATE TABLE") + strlen(createQuery.tableName) + 1, " ");
        while (token) {
            strcpy(createQuery.columns[createQuery.columnCount++].columnName, token);
            token = strtok(NULL, " ");
        }
        createTable(&createQuery);
    } else if (commandType == INSERT) {
        InsertQuery insertQuery;
        sscanf(command, "INSERT INTO %s VALUES (%[^)])", insertQuery.tableName, insertQuery.values[0]);
        insertQuery.valueCount = 0;

        char *value = strtok(insertQuery.values[0], ",");
        while (value) {
            while (*value == ' ') value++; // Trim whitespace
            strcpy(insertQuery.values[insertQuery.valueCount++], value);
            value = strtok(NULL, ",");
        }
        insertRow(&insertQuery);
    } else if (commandType == SELECT) {
        SelectQuery selectQuery;
        sscanf(command, "SELECT * FROM %s", selectQuery.tableName);
        selectQuery.hasAnd = strstr(command, "AND") != NULL;
        selectQuery.hasOr = strstr(command, "OR") != NULL;
        selectQuery.condition1[0] = '\0';
        selectQuery.condition2[0] = '\0';

        char *whereClause = strstr(command, "WHERE");
        if (whereClause) {
            char *conditions = whereClause + strlen("WHERE") + 1;
            char *andPos = strstr(conditions, "AND");
            char *orPos = strstr(conditions, "OR");

            if (andPos) {
                *andPos = '\0'; // Terminate first condition
                strcpy(selectQuery.condition1, conditions);
                strcpy(selectQuery.condition2, andPos + strlen("AND") + 1);
            } else if (orPos) {
                *orPos = '\0'; // Terminate first condition
                strcpy(selectQuery.condition1, conditions);
                strcpy(selectQuery.condition2, orPos + strlen("OR") + 1);
            } else {
                strcpy(selectQuery.condition1, conditions);
            }
        }
        selectRows(&selectQuery);
    } else if (commandType == UPDATE) {
        UpdateQuery updateQuery;
        sscanf(command, "SET %s %[^=]=%s WHERE %[^=]=%s",
               updateQuery.tableName,
               updateQuery.columnName,
               updateQuery.newValue,
               updateQuery.conditionColumn1,
               updateQuery.conditionValue1);

        updateQuery.hasAndCondition = strstr(command, "AND") != NULL;
        updateQuery.hasOrCondition = strstr(command, "OR") != NULL;

        if (updateQuery.hasAndCondition || updateQuery.hasOrCondition) {
            sscanf(command, "SET %s %[^=]=%s WHERE %[^=]=%s AND %[^=]=%s",
                   updateQuery.tableName,
                   updateQuery.columnName,
                   updateQuery.newValue,
                   updateQuery.conditionColumn1,
                   updateQuery.conditionValue1,
                   updateQuery.conditionColumn2,
                   updateQuery.conditionValue2);
        } else {
            strcpy(updateQuery.conditionColumn2, ""); // No second condition
        }
        updateRow(&updateQuery);
    } else if (commandType == DROP) {
        DropQuery dropQuery;
        sscanf(command, "DROP TABLE %s", dropQuery.tableName);
        dropTable(&dropQuery);
    } else if (commandType == DELETE) {
    DeleteQuery deleteQuery;
    deleteQuery.conditionColumn2[0] = '\0'; // Default for no second condition

    // Parse the basic DELETE command
    sscanf(command, "DELETE FROM %s WHERE %[^=]=%s", deleteQuery.tableName, deleteQuery.conditionColumn, deleteQuery.conditionValue);

    // Check for AND condition
    deleteQuery.hasAnd = strstr(command, "AND") != NULL;

    if (deleteQuery.hasAnd) {
        sscanf(strstr(command, "AND") + strlen("AND"), "%[^=]=%s", deleteQuery.conditionColumn2, deleteQuery.conditionValue2);
    }

    // Now call the DeleteRow function
    DeleteRow(&deleteQuery);
    }
    else if (strncmp(command, "SHOW TABLE", 10) == 0) {
    ShowQuery showQuery;
    sscanf(command, "SHOW TABLE %s", showQuery.tableName);
    showQuery.columnCount = 0;

    char *columnsPart = strstr(command, "COLUMNS");
    if (columnsPart) {
        char *token = strtok(columnsPart + strlen("COLUMNS"), ", ");
        while (token && showQuery.columnCount < MAX_COLUMNS) {
            strcpy(showQuery.columnNames[showQuery.columnCount++], token);
            token = strtok(NULL, ", ");
        }
    } else {
        // If no columns specified, use all columns from the table
        for (int i = 0; i < tableCount; i++) {
            if (strcmp(database[i].tableName, showQuery.tableName) == 0) {
                showQuery.columnCount = database[i].columnCount;
                for (int j = 0; j < showQuery.columnCount; j++) {
                    strcpy(showQuery.columnNames[j], database[i].columns[j].columnName);
                }
                break;
            }
        }
    }

    showTable(&showQuery);
    }
    else if (strncmp(command, "CREATE INDEX", 12) == 0) {
    IndexQuery indexQuery;
    sscanf(command, "CREATE INDEX %s ON %s", indexQuery.indexName, indexQuery.tableName);
    indexQuery.columnCount = 0;

    char *columnsPart = strstr(command, "(");
    if (columnsPart) {
        char *token = strtok(columnsPart + 1, ",)");
        while (token && indexQuery.columnCount < MAX_COLUMNS) {
            strcpy(indexQuery.columnNames[indexQuery.columnCount++], token);
            token = strtok(NULL, ",");
        }
    }
    createIndex(&indexQuery);
    } else if (strncmp(command, "DROP INDEX", 10) == 0) {
    char indexName[MAX_NAME_LENGTH];
    sscanf(command, "DROP INDEX %s", indexName);
    dropIndex(indexName);
    }
    else if (strncmp(command, "LIST TABLES", 12) == 0) {
        ListAllTables();
    }


    else {
        printf("Unknown command: %s\n", command);
        }
    }

void showTable(ShowQuery *showQuery) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(database[i].tableName, showQuery->tableName) == 0) {
            printf("\nTable: %s\n", showQuery->tableName);

            // Determine column widths
            int widths[MAX_COLUMNS] = {0};
            for (int c = 0; c < showQuery->columnCount; c++) {
                widths[c] = strlen(showQuery->columnNames[c]);
            }
            for (int j = 0; j < database[i].rowCount; j++) {
                for (int c = 0; c < showQuery->columnCount; c++) {
                    for (int k = 0; k < database[i].columnCount; k++) {
                        if (strcmp(showQuery->columnNames[c], database[i].columns[k].columnName) == 0) {
                            int len = strlen(database[i].rows[j].values[k]);
                            if (len > widths[c]) {
                                widths[c] = len; // Update width if necessary
                            }
                        }
                    }
                }
            }
            // Print separator
            printf("+");
            for (int c = 0; c < showQuery->columnCount; c++) {
                for (int k = 0; k < widths[c] + 2; k++) {
                    printf("-");
                }
                printf("+");
            }
            printf("\n");

            // Print header
            printf("|");
            for (int c = 0; c < showQuery->columnCount; c++) {
                printf("%-*s|", widths[c] + 2, showQuery->columnNames[c]);
            }
            printf("\n");

            // Print separator
            printf("+");
            for (int c = 0; c < showQuery->columnCount; c++) {
                for (int k = 0; k < widths[c] + 2; k++) {
                    printf("-");
                }
                printf("+");
            }
            printf("\n");

            // Print rows
            for (int j = 0; j < database[i].rowCount; j++) {
                printf("|");
                for (int c = 0; c < showQuery->columnCount; c++) {
                    for (int k = 0; k < database[i].columnCount; k++) {
                        if (strcmp(showQuery->columnNames[c], database[i].columns[k].columnName) == 0) {
                            printf(" %-*s |", widths[c], database[i].rows[j].values[k]);
                        }
                    }
                }
                printf("\n");

                // Print row separator
                printf("+");
                for (int c = 0; c < showQuery->columnCount; c++) {
                    for (int k = 0; k < widths[c] + 2; k++) {
                        printf("-");
                    }
                    printf("+");
                }
                printf("\n");
            }

            return;
        }
    }
    printf("Table %s not found.\n", showQuery->tableName);
}

void ListAllTables() {
    printf("\nList of all tables:\n");
    printf("+----------------+---------+-------------+\n");
    printf("| Table Name     | ROWS    |  Columns    |\n");
    printf("+----------------+---------+-------------+\n");

    for (int i = 0; i < tableCount; i++) {
        printf("| %-14s | %-7d | %-7d     |\n",
               database[i].tableName,
               database[i].rowCount,
               database[i].columnCount);
    }

    printf("+----------------+---------+-------------+\n");

    // Prompt user for a specific table
    char tableName[MAX_NAME_LENGTH];
    printf("Enter the name of the table you want to view (or 'exit' to cancel): ");
    scanf("%s", tableName);

    // Check if the user wants to exit
    if (strcmp(tableName, "exit") == 0) {
        printf("Exiting table view.\n");
        return;
    }

    // Prepare the show query
    ShowQuery showQuery;
    strcpy(showQuery.tableName, tableName);


    //process the command we'll initiallized to show the user selected table
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "SHOW TABLE %s", showQuery.tableName);

    processCommand(&command);
}

void WriteCommand() {
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        printf("Enter the command (or type -EXIT to quit): ");
        getchar(); // Consume the newline character left by previous scanf
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        // Remove newline character if it exists
        command[strcspn(command, "\n")] = 0;

        // Check if the user wants to exit
        if (strcmp(command, "-EXIT") == 0) {
            printf("Exiting command input.\n");
            break;
        }

        // Process the command
        processCommand(command);
    }
}

int GetColumnCount(char *tableName){
    int count = -1;

    for(int i=0;i<tableCount;i++)
    {
        if(strcmp(&database[i].tableName,tableName)==0)
        {
            count = database[i].columnCount;
            break;
        }
    }
    return count;
}

int GetRowCount(char *tableName){
    int count = -1;

    for(int i=0;i<tableCount;i++)
    {
        if(strcmp(&database[i].tableName,tableName)==0)
        {
            count = database[i].rowCount;
            break;
        }
    }
    return count;
}


