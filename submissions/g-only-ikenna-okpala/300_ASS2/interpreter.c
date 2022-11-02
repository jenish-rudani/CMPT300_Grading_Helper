#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void readCommand(char * command, int length){
    fgets(command, length, stdin);
}

bool validateCommand(char * command){
    char splitChar[200];
    strcpy(splitChar, command);

    char * split = strtok(splitChar, " ");

    if(strcmp(split, "s-talk") != 0){
        return false;
    }

    int count = 0;
    while(split != NULL){
        split = strtok(NULL, " ");
        count++;
    }

    if(count != 4){
        return false;
    }

    return true;
}

void splitCommand(char ** machineInfo, char * command){
    strtok(command, " ");
    machineInfo[0] = strtok(NULL, " ");
    machineInfo[1] = strtok(NULL, " ");
    machineInfo[2] = strtok(NULL, " ");

    int num = atoi(machineInfo[2]);
    
    snprintf(machineInfo[2], 10, "%d", num);

}