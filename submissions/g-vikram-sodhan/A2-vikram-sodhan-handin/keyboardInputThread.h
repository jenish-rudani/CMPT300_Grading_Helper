#ifndef _KEYBOARD_INPUT_THREAD_H_
#define _KEYBOARD_INPUT_THREAD_H_

void keyboardInputThread_init(void);
void* keyboardInput(void* unused);
void keyboardInputThread_shutdown(void);

#endif