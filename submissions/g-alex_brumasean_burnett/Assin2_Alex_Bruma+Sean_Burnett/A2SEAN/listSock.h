#ifndef _LISTSOCK_H
#define _LISTSOCK_H
#include "list.h"
void getInputList();

int getoutputListMutex();

int getinputListMutex();

int List_init();

void* popInputList();
//pops from the back 
void* popOutputList();
//pushes from the front
int pushtoInputList(void* pItem);

int pushtoOutputList(void* pItem);

int pO();

int pI();

int vO();

int vI();

int pbuffI();

int pbuffO();

int pitemI();

int pitemO();

int vbuffI();

int vbuffO();

int vitemI();

int vitemO();

int breaks();

#endif