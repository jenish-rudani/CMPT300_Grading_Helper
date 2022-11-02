#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdbool.h>
void readCommand(char * command, int length);

bool validateCommand(char * command);

void splitCommand(char ** machineInfo, char * command);

#endif