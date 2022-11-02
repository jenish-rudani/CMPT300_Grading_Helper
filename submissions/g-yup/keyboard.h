#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

void Keyboard_init();
void Keyboard_addMessageInList(char* message);
char* Keyboard_pickMessageInList();
void Keyboard_cancel();
void Keyboard_waitForShutdown();

#endif