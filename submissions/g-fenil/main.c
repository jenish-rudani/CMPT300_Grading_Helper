#include <stdio.h>
#include "list.h"

int main(){
    printf ("Hello \n");
     List* example= List_create();
     int count = List_count(example);
     //printf ("Count as of now is %d \n", count);

     int anyNum = 5;
     void* ptItem = &anyNum;
     List_append(example,ptItem);
    //  count = List_count(example);
    //  printf ("Count as of now is %d \n", count);
     int anyNum2 = 6;
     void* ptItem2 = &anyNum2;
     List_insert_after(example,ptItem2);
     count = List_count(example);
     printf ("Count as of now is %d \n", count);

    void* currentItem = List_curr(example);
    printf("Current Item is: %d \n", *(int*)currentItem);
    
    List_prev(example);
    currentItem = List_curr(example);
    printf("Current Item is: %d \n", *(int*)currentItem);

    int anyNum3 = 7;
    void* ptItem3 = &anyNum3;
    //List_insert_before(example,ptItem3);
    List_insert_before(example,ptItem3);
    // List_first(example);
    // currentItem = List_curr(example);
    // printf("first Item is: %d \n", *(int*) currentItem);
    
    
    List_trim(example);
    // count = List_count(example);
    // printf ("Count as of now is %d \n", count);
    

    // List_first(example);
    // currentItem = List_curr(example);
    // printf("first Item is: %d \n", *(int*) currentItem);

    
   
    List* example2 = List_create();
    int anyNum12 = 12;
    void* ptItem12 = &anyNum12;
    List_append(example2,ptItem12);
    int anyNum13 = 13;
    void* ptItem13 = &anyNum13;
    List_append(example2,ptItem13);
    int anyNum14 = 14;
    void* ptItem14 = &anyNum14;
    List_append(example2,ptItem14);

    List_concat(example,example2);
    // List_last(example);
    // currentItem = List_curr(example);
    // printf("last Item is: %d \n", *(int*) currentItem);

    // currentItem = List_curr(example2);
    // printf("Current Item is: %d \n", *(int*)currentItem);
    List_first(example);
    for(int i=0; i<example->NodeCount; i++){
        void* currentItem = List_curr(example);
        printf("Item are: %d \n", *(int*) currentItem);
        List_next(example);
    }
    return 0;
}