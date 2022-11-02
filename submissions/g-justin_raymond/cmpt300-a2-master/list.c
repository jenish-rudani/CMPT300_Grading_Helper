#include "includes/list.h"
#include <stdlib.h>

/* Global Variables */
static Node nodePool[LIST_MAX_NUM_NODES];
static List listPool[LIST_MAX_NUM_HEADS];
static List* deletedLists[LIST_MAX_NUM_HEADS];
static Node* deletedNodes[LIST_MAX_NUM_NODES];
static int nextList = 0;
static int nextNode = 0;
static int deletedListsCount = 0;
static int deletedNodesCount = 0;

/* Local Headers */
Node* Node_availableNode();

/*Function Definitions*/
// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create(){
    if ((nextNode == LIST_MAX_NUM_NODES && deletedNodesCount == 0) || (nextList == LIST_MAX_NUM_HEADS && deletedListsCount == 0)) // if none left, failure
        return NULL;
    // should be able to make a new list 
    // find out whether to use from listPool or deletedLists
    List* newList;
    if (nextList < LIST_MAX_NUM_HEADS){ //use listPool
        newList = listPool + nextList;
        nextList++;
    }
    else{   // use deletedLists
        newList = deletedLists[deletedListsCount - 1];
        deletedListsCount--;
    }
    newList->count = 0;
    newList->head = NULL;
    newList->tail = NULL;
    newList->currentNode = NULL;
    return newList;
};

// Returns the number of items in pList.
int List_count(List* pList){
    return pList->count;
};

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList){
    if (pList->count == 0){
        pList->currentNode = NULL;
        pList->currentIndex = -1;
    }
    else{
        pList->currentNode = pList->head;
        pList->currentIndex = 0;
    }
    
    return (pList->count == 0)? NULL : pList->head->item;
};

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList){
    if (pList->count == 0){
        pList->currentNode = NULL;
        pList->currentIndex = pList->count;
    }
    else{
        pList->currentNode = pList->tail;
        pList->currentIndex = pList->count - 1;
    }

    return (pList->count == 0)? NULL : pList->tail->item;
}; 

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList){
    (pList->currentIndex)++; //advances by one Node length
    if ((pList->currentIndex - 1) == (pList->count - 1)){ //check if current == tail
        pList->currentNode = NULL;
        pList->currentIndex = pList->count;
        return NULL;
    }
    
    pList->currentNode = pList->currentNode->next;
    return pList->currentNode->item;
};

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList){
    pList->currentNode = pList->currentNode->prev;
    (pList->currentIndex)--; //goes backwards by one Node length
    if ((pList->currentIndex + 1) == 0){ //check if current == head
        pList->currentNode = NULL;
        pList->currentIndex = -1;
        return NULL;
    }
    return pList->currentNode->item;
};

// Returns a pointer to the current item in pList.
void* List_curr(List* pList){
    if (pList->currentNode == NULL)
        return NULL;
    return pList->currentNode->item;
};

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem){
    //check if there are any nodes available
    Node* newNode = Node_availableNode();
    if (!newNode) //if NULL, failure
        return LIST_FAIL;

    newNode->item = pItem;
    
    //redefine all next/prev relations
    if (pList->count == 0){
        newNode->next = NULL;
        newNode->prev = NULL;
        pList->head = newNode;
        pList->tail = newNode;
        pList->currentIndex = -1; //fixes index
    }
    else if (pList->currentIndex == -1){ //is before, redefine head
        newNode->next = pList->head;
        newNode->prev = NULL;
        pList->head->prev = newNode;
        pList->head = newNode;
    }
    else if (pList->currentIndex == pList->count || pList->currentIndex == pList->count - 1){ //is after or at end, make sure that when beyond, keep pointer at end
        newNode->prev = pList->currentNode;
        newNode->next = NULL;
        pList->currentNode->next = newNode;
        pList->tail = newNode;
        pList->currentIndex--; //fix index
    }
    else{
        newNode->next = pList->currentNode->next;
        newNode->prev = pList->currentNode;
        pList->currentNode->next->prev = newNode;
        pList->currentNode->next = newNode;
    }
    
    pList->currentNode = newNode;    
    pList->currentIndex++;
    pList->count++;
    return LIST_SUCCESS;
};

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem){
    //check if there are any nodes available
    Node* newNode = Node_availableNode();
    if (!newNode) //if NULL, failure
        return LIST_FAIL;

    newNode->item = pItem;
    
    //redefine all next/prev relations
    if (pList->count == 0){
        newNode->next = NULL;
        newNode->prev = NULL;
        pList->head = newNode;
        pList->tail = newNode;
        pList->currentIndex = 1; //fix index
    }
    else if (pList->currentIndex == -1){ //is before
        newNode->next = pList->head;
        newNode->prev = NULL;
        pList->head->prev = newNode;
        pList->head = newNode;
        pList->currentIndex = 1; //fix index
    }
    else if (pList->currentIndex == pList->count){ //is after, make sure that when beyond, keep pointer at end
        newNode->prev = pList->currentNode;
        newNode->next = NULL;
        pList->currentNode->next = newNode;
        pList->currentIndex++;
    }
    else{
        newNode->next = pList->currentNode;
        newNode->prev = pList->currentNode->prev;
        pList->currentNode->prev = newNode;
        pList->currentNode->prev->next = newNode;
    }
    
    pList->currentNode = newNode;    
    pList->currentIndex--;
    pList->count++;
    return LIST_SUCCESS;
};

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem){
    //check if there are any nodes available
    Node* newNode = Node_availableNode();
    if (!newNode) //if NULL, failure
        return LIST_FAIL;

    newNode->item = pItem;
    
    if (pList->count != 0){
        newNode->prev = pList->tail;
        newNode->next = NULL;
        pList->tail->next = newNode;
    }
    else{
        newNode->prev = NULL;
        pList->head = newNode;
    }
    pList->tail = newNode;   
    newNode->next = NULL;
    
    pList->currentNode = newNode;    
    pList->currentIndex = pList->count;
    pList->count++;
    return LIST_SUCCESS;
};

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem){
    //check if there are any nodes available
    Node* newNode = Node_availableNode();
    if (!newNode) //if NULL, failure
        return LIST_FAIL;

    newNode->item = pItem;
    
    if (pList->count != 0){
        newNode->next = pList->head;
        pList->head->prev = newNode; 
    }
    else{
        newNode->next = NULL;
        pList->tail = newNode;
    }

    newNode->prev = NULL;
    pList->head = newNode;
    
    pList->currentNode = newNode;    
    pList->currentIndex = 0;
    pList->count++;
    return LIST_SUCCESS;
};

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList){
    if (pList->count == 0)
        return NULL;
    if (pList->currentIndex == -1 || pList->currentIndex == pList->count)
        return NULL;
    
    void* returnItem = pList->currentNode->item;
    Node* newDeletedNode = pList->currentNode;

    if (pList->count == 1){ //if deleting the only node of pList
        pList->head = NULL;
        pList->tail = NULL;
        pList->currentNode = NULL;
    }
    else if (pList->currentIndex == 0){ //need to redefine head
        pList->currentNode = pList->currentNode->next;
        pList->head = pList->currentNode;
        pList->head->prev = NULL;
    }
    else if (pList->currentIndex == pList->count - 1){ //need to redefine tail
        pList->currentNode = pList->currentNode->prev;
        pList->tail = pList->currentNode;
        pList->tail->next = NULL;
    }
    else{
        pList->currentNode->prev->next = pList->currentNode->next;
        pList->currentNode->next->prev = pList->currentNode->prev;
        pList->currentNode = pList->currentNode->next;
    }
    deletedNodes[deletedNodesCount] = newDeletedNode;
    deletedNodesCount++;
    pList->count--;
    pList->currentIndex--;
    return returnItem;
};

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList){
    if (pList->count == 0)
        return NULL;

    void* itemReturn = pList->tail->item;
    pList->currentNode = pList->tail;
    Node* newDeletedNode = pList->currentNode;

    if (pList->count == 1){ // head is NULL
        pList->head = NULL;
        pList->tail = NULL;
        pList->currentNode = NULL;
    }
    else if (pList->count == 2){ // remaining node is head AND tail
        pList->tail = pList->head;
        pList->head->next = NULL;
        pList->currentNode = pList->currentNode->prev;
    }
    else{
        pList->currentNode = pList->currentNode->prev;
        pList->tail = pList->currentNode;
        pList->currentNode->next = NULL;
    }

    deletedNodes[deletedNodesCount] = newDeletedNode;
    deletedNodesCount++;
    pList->currentIndex--;
    pList->count--;

    return itemReturn;
};

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2){
    pList2->head->prev = pList1->tail;
    pList1->tail->next = pList2->head;

    pList1->tail = pList2->tail;
    pList1->count += pList2->count;
    
    pList2->count = 0;
    pList2->currentIndex = -1;
    pList2->currentNode = NULL;
    pList2->head = NULL;
    pList2->tail = NULL;
    
    deletedLists[deletedListsCount] = pList2;
    deletedListsCount++;
};

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    pList->currentIndex = pList->count - 1;
    pList->currentNode = pList->tail;
    while (pList->count != 0){
        pItemFreeFn(pList->currentNode->item); //free the item
        List_trim(pList); //will cut the tail, and update currentNode
    }
    
    pList->count = 0;
    pList->currentIndex = -1;
    pList->currentNode = NULL;
    pList->head = NULL;
    pList->tail = NULL;

    deletedLists[deletedListsCount] = pList;
    deletedListsCount++;
};

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    if (pList->currentIndex == -1)
        pList->currentIndex = 0;

    for (int i = pList->currentIndex; i < pList->count; i++){
        if (pComparator(pList->currentNode->item, pComparisonArg))
            return pList->currentNode->item;
        pList->currentNode = pList->currentNode->next;
        pList->currentIndex++;
    }

    return NULL;
};

/*MY FUNCTIONS v */
// Returns the pointer to the next available node, returns NULL if failure
Node* Node_availableNode(){
    Node* newNode;
    if (nextNode >= LIST_MAX_NUM_NODES) //use deletedNodes List
        if (deletedNodesCount == 0)
            return NULL;
        else{
            newNode = deletedNodes[deletedNodesCount-- - 1];
        }
    else{ // use nodePool
        newNode = nodePool + nextNode;
        nextNode++;
    }
    return newNode;
};
/*MY FUNCTIONS ^ */