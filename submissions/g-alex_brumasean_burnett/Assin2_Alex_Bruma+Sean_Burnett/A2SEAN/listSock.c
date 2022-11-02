#include "listSock.h"
#include "list.h"

List* inputList;
List* outputList;
int inputListMutex;
int outputListMutex;
int buffAvailO;
int BREAK;
int itemAvailO;
int buffAvailI;
int itemAvailI;



int getinputListMutex(){
    return inputListMutex;
}

int getoutputListMutex(){
    return outputListMutex;
}

int List_init(){
    inputList = List_create();
    outputList = List_create();
    inputListMutex = 1;
    outputListMutex = 1;
    buffAvailO = 100;
    itemAvailO = 0;
    buffAvailI = 100;
    itemAvailI = 0;
    BREAK = 0;
    return 0;
}

void* popInputList(){
    return List_trim(inputList);
}

void* popOutputList(){
    return List_trim(outputList);
}

int pushtoInputList(void* pItem){
    List_prepend(inputList, pItem);
    return 0;
}

int pushtoOutputList(void* pItem){
    List_prepend(outputList,pItem);
    return 0;
}




int pO(){
    while(outputListMutex == 0);
    outputListMutex--;
    return 0;
}

int pI(){
    while(inputListMutex == 0);
    inputListMutex--;
    return 0;
}
int pbuffI(){
    while(buffAvailI == 0);
    buffAvailI--;
    return 0;
}
int pbuffO(){
    while(buffAvailO == 0);
    buffAvailO--;
    return 0;
}

int pitemI(){
    while(itemAvailI == 0 & BREAK == 0);
    itemAvailI--;
    return 0;
}

int pitemO(){
    while(itemAvailO == 0 & BREAK == 0);
    itemAvailO--;
    return 0;
}


int vO(){
    outputListMutex++;
    return 0;
}

int vI(){
    inputListMutex++;
    return 0;
}
int vbuffI(){
    buffAvailI++;
    return 0;
}
int vbuffO(){
    buffAvailO++;
    return 0;
}

int vitemI(){
    itemAvailI++;
    return 0;
}

int vitemO(){
    itemAvailO++;
    return 0;
}
int breaks(){
    BREAK++;
    return 0;
}

