#include "list.h"

#include <assert.h>
#include <stdio.h>

// Statically allocated arrays to store list nodes and list heads
static Node NodesArray[LIST_MAX_NUM_NODES];
static List HeadsArray[LIST_MAX_NUM_HEADS];

// Indicies of the next free list node and free list head in their respective arrays
// This is used in combination with nextFreeNodeIndex and nextFreeListIndex to handle memory
// allocation in O(1) time
static int FreeNodeIndex = -1;
static int FreeHeadIndex = -1;

// To check if this is the first call for List_create where we can do O(n) operations to setup
// Set to false after first initialization and never change to true again
static bool FirstInitialization = true;

// Helper function to handle the initial setup required for List_create - O(n) operations
static void initialSetup() {
    // Setup the NodesArray with Empty nodes that have the nextFreeNode to be the next node in the array
    // The last node in the will have the nextFreeNode to point back to the first item
    for (int i = 0; i < LIST_MAX_NUM_NODES; i++) {
        NodesArray[i].nodeArrayIndex = i;
        NodesArray[i].itemPtr = NULL;
        NodesArray[i].nextNode = NULL;
        NodesArray[i].prevNode = NULL;
        NodesArray[i].nextFreeNodeIndex = (i == LIST_MAX_NUM_NODES - 1) ? 0 : i + 1;
        NodesArray[i].taken = false;
    }

    // Setup the HeadsArray with Empty List that have the nextListNode to be the next List in the array
    // The last List in the will have the nextListNode to point back to the first item
    for (int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
        HeadsArray[i].currentItemPtr = NULL;
        HeadsArray[i].currentNodePtr = NULL;
        HeadsArray[i].listHeadNodePtr = NULL;
        HeadsArray[i].listTailNodePtr = NULL;
        HeadsArray[i].totalItemsInList = 0;
        HeadsArray[i].headArrayIndex = i;
        HeadsArray[i].nextFreeListIndex = (i == LIST_MAX_NUM_HEADS - 1) ? 0 : i + 1;
        HeadsArray[i].currentNodePtrStatus = LIST_OOB_START;
        HeadsArray[i].taken = false;
    }

    // Set the first free node to be at the start of the arrays
    FreeNodeIndex = 0;
    FreeHeadIndex = 0;

    FirstInitialization = false;
}

static bool isFreeHeadArrayFull() {
    return HeadsArray[FreeHeadIndex].taken;
}

static bool isFreeNodeArrayFull() {
    return NodesArray[FreeNodeIndex].taken;
}

static void freeNode(Node* node) {
    // Only free the node not the itemPtr. The itemPtr will be handled by the user

    // Free node
    node->nextNode = NULL;
    node->prevNode = NULL;
    node->taken = false;

    // Set the nextFreeNodeIndex to be the current FreeNodeIndex
    node->nextFreeNodeIndex = FreeNodeIndex;

    // Set the FreeNodeIndex to be the newly free Node
    FreeNodeIndex = node->nodeArrayIndex;
}

static void freeList(List* List) {
    // Free the list head so we can use it in the future.
    List->currentItemPtr = NULL;
    List->currentNodePtr = NULL;
    List->listHeadNodePtr = NULL;
    List->listTailNodePtr = NULL;
    List->totalItemsInList = 0;
    List->currentNodePtrStatus = LIST_OOB_START;
    List->taken = false;

    // returning the head back to the array to be able to re-use again
    // Set the nextFreeListIndex to be the current FreeHeadIndex
    List->nextFreeListIndex = FreeHeadIndex;

    // Set the FreeHeadIndex to be the newly free Head
    FreeHeadIndex = List->headArrayIndex;
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List* List_create() {
    if (FirstInitialization) {
        printf("Initial List Setup \n");
        initialSetup();
    }

    if (isFreeHeadArrayFull()) {
        return NULL;
    }

    // Since using List ptr need to use & to get location address
    List* newList = &HeadsArray[FreeHeadIndex];

    // Set the FreeHeadIndex to become whatever the now taken list has saved as it's nextFreeListIndex
    FreeHeadIndex = HeadsArray[FreeHeadIndex].nextFreeListIndex;
    newList->taken = true;

    return newList;
}

// Returns the number of items in pList.
int List_count(List* pList) {
    assert(pList != NULL);
    return pList->totalItemsInList;
}

// Returns a pointer to the first item in pList and makes the first item the
// current item. Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    assert(pList != NULL);
    // empty list
    if (pList->totalItemsInList == 0) {
        pList->currentItemPtr = NULL;
        pList->currentNodePtr = NULL;
        return NULL;
    }

    // non-empty list
    pList->currentNodePtr = pList->listHeadNodePtr;          // Set current node to list head node
    pList->currentItemPtr = pList->currentNodePtr->itemPtr;  // Set current item to list head item

    return pList->currentItemPtr;
}

// Returns a pointer to the last item in pList and makes the last item the
// current item. Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    assert(pList != NULL);
    // empty list
    if (pList->totalItemsInList <= 0) {
        pList->currentItemPtr = NULL;
        pList->currentNodePtr = NULL;
        return NULL;
    }

    // non-empty list
    pList->currentNodePtr = pList->listTailNodePtr;          // Set current node to list tail node
    pList->currentItemPtr = pList->currentNodePtr->itemPtr;  // Set current item to list tail item

    return pList->currentItemPtr;
}

// Advances pList's current item by one, and returns a pointer to the new
// current item. If this operation advances the current item beyond the end of
// the pList, a NULL pointer is returned and the current item is set to be
// beyond end of pList.
void* List_next(List* pList) {
    assert(pList != NULL);
    // check if currentItem is already before or beyond the list (currentItemPtr == Null)
    if (pList->currentItemPtr == NULL) {
        // currentNodePtrStatus should already be set so don't change anything
        return NULL;
    }

    // check if current node is tail node
    if (pList->currentNodePtr == pList->listTailNodePtr) {
        pList->currentNodePtr = NULL;
        pList->currentItemPtr = NULL;
        pList->currentNodePtrStatus = LIST_OOB_END;
        return NULL;
    }

    // we have valid current item and valid next current item

    pList->currentNodePtr = pList->currentNodePtr->nextNode;
    pList->currentItemPtr = pList->currentNodePtr->itemPtr;

    return pList->currentItemPtr;
}

// Backs up pList's current item by one, and returns a pointer to the new
// current item. If this operation backs up the current item beyond the start of
// the pList, a NULL pointer is returned and the current item is set to be
// before the start of pList.
void* List_prev(List* pList) {
    assert(pList != NULL);
    // check if currentItem is already before or beyond the list (currentItemPtr == Null)
    if (pList->currentItemPtr == NULL) {
        // currentNodePtrStatus should already be set so don't change anything
        return NULL;
    }

    // check if current node is head node
    if (pList->currentNodePtr == pList->listHeadNodePtr) {
        pList->currentNodePtr = NULL;
        pList->currentItemPtr = NULL;
        pList->currentNodePtrStatus = LIST_OOB_START;
        return NULL;
    }

    // we have valid current item and valid prev current item

    pList->currentNodePtr = pList->currentNodePtr->prevNode;
    pList->currentItemPtr = pList->currentNodePtr->itemPtr;

    return pList->currentItemPtr;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
    assert(pList != NULL);
    return pList->currentItemPtr;
}

// Adds the new item to pList directly after the current item, and makes item
// the current item. If the current pointer is before the start of the pList,
// the item is added at the start. If the current pointer is beyond the end of
// the pList, the item is added at the end. Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem) {
    assert(pList != NULL && pItem != NULL);
    // If there is no free node for the new item failure
    if (isFreeNodeArrayFull()) {
        return -1;
    }

    // Create/get free node
    Node* newNode = &NodesArray[FreeNodeIndex];
    FreeNodeIndex = newNode->nextFreeNodeIndex;
    newNode->itemPtr = pItem;
    newNode->taken = true;

    if (pList->totalItemsInList == 0) {
        // list is empty so node will be only item in the list
        pList->listHeadNodePtr = newNode;
        pList->listTailNodePtr = newNode;
    }

    // check if currentItem is already before or beyond the list (currentItemPtr == Null)
    else if (pList->currentItemPtr == NULL) {
        if (pList->currentNodePtrStatus == LIST_OOB_START) {
            // add item to start of pList
            newNode->nextNode = pList->listHeadNodePtr;
            pList->listHeadNodePtr->prevNode = newNode;
            pList->listHeadNodePtr = newNode;

        } else {
            // pList->currentNodePtrStatus will be LIST_OOB_END so add item to end of pList
            newNode->prevNode = pList->listTailNodePtr;
            pList->listTailNodePtr->nextNode = newNode;
            pList->listTailNodePtr = newNode;
        }
    }

    else {
        // currentItem is not Null

        // if the current node is the tail node then set the newly insert node as the tail
        if (pList->currentNodePtr == pList->listTailNodePtr) {
            pList->listTailNodePtr = newNode;
        } else {
            // if current is not the tail set the next and prev for the neighbouring nodes
            newNode->nextNode = pList->currentNodePtr->nextNode;
            pList->currentNodePtr->nextNode->prevNode = newNode;
        }

        // set the prev the new node and the next for the current node
        newNode->prevNode = pList->currentNodePtr;

        pList->currentNodePtr->nextNode = newNode;
    }

    // setting newNode to be the current item
    pList->currentNodePtr = newNode;
    pList->currentItemPtr = newNode->itemPtr;
    pList->totalItemsInList++;
    return 0;
}

// Adds item to pList directly before the current item, and makes the new item
// the current one. If the current pointer is before the start of the pList, the
// item is added at the start. If the current pointer is beyond the end of the
// pList, the item is added at the end. Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem) {
    assert(pList != NULL && pItem != NULL);

    // If there is no free node for the new item failure
    if (isFreeNodeArrayFull()) {
        return -1;
    }

    // Create/get free node
    Node* newNode = &NodesArray[FreeNodeIndex];
    FreeNodeIndex = newNode->nextFreeNodeIndex;
    newNode->itemPtr = pItem;
    newNode->taken = true;

    if (pList->totalItemsInList == 0) {
        // list is empty so node will be only item in the list
        pList->listHeadNodePtr = newNode;
        pList->listTailNodePtr = newNode;
    }

    // check if currentItem is already before or beyond the list (currentItemPtr == Null)
    else if (pList->currentItemPtr == NULL) {
        if (pList->currentNodePtrStatus == LIST_OOB_START) {
            // add item to start of pList
            newNode->nextNode = pList->listHeadNodePtr;
            pList->listHeadNodePtr->prevNode = newNode;
            pList->listHeadNodePtr = newNode;

        } else {
            // pList->currentNodePtrStatus will be LIST_OOB_END so add item to end of pList
            newNode->prevNode = pList->listTailNodePtr;
            pList->listTailNodePtr->nextNode = newNode;
            pList->listTailNodePtr = newNode;
        }
    }

    else {
        // currentItem is not Null
        // if the current node is the head node then set the newly insert node as the head
        if (pList->currentNodePtr == pList->listHeadNodePtr) {
            pList->listHeadNodePtr = newNode;
        } else {
            // if current is not the head set the prev and next for the neighbouring nodes
            newNode->prevNode = pList->currentNodePtr->prevNode;
            pList->currentNodePtr->prevNode->nextNode = newNode;
        }

        // set the next for new node and the prev for the current node
        newNode->nextNode = pList->currentNodePtr;
        pList->currentNodePtr->prevNode = newNode;
    }

    // setting newNode to be the current item
    pList->currentNodePtr = newNode;
    pList->currentItemPtr = newNode->itemPtr;
    pList->totalItemsInList++;
    return 0;
}

// Adds item to the end of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
    assert(pList != NULL && pItem != NULL);
    // If there is no free node for the new item failure
    if (isFreeNodeArrayFull()) {
        return -1;
    }

    // Create/getfree node
    Node* newNode = &NodesArray[FreeNodeIndex];
    FreeNodeIndex = newNode->nextFreeNodeIndex;
    newNode->itemPtr = pItem;
    newNode->taken = true;

    if (pList->totalItemsInList == 0) {
        // list is empty so node will be only item in the list
        pList->listHeadNodePtr = newNode;
        pList->listTailNodePtr = newNode;
    } else {
        // any other case add Node to the end of pList
        newNode->prevNode = pList->listTailNodePtr;
        pList->listTailNodePtr->nextNode = newNode;
        pList->listTailNodePtr = newNode;
    }

    // setting newNode to be the current item
    pList->currentNodePtr = newNode;
    pList->currentItemPtr = newNode->itemPtr;
    pList->totalItemsInList++;
    return 0;
}

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
    assert(pList != NULL && pItem != NULL);
    // If there is no free node for the new item failure
    if (isFreeNodeArrayFull()) {
        return -1;
    }

    // Create/get free node
    Node* newNode = &NodesArray[FreeNodeIndex];
    FreeNodeIndex = newNode->nextFreeNodeIndex;
    newNode->itemPtr = pItem;
    newNode->taken = true;

    if (pList->totalItemsInList == 0) {
        // list is empty so node will be only item in the list
        pList->listHeadNodePtr = newNode;
        pList->listTailNodePtr = newNode;
    } else {
        // any other case add Node to the front of pList
        newNode->nextNode = pList->listHeadNodePtr;
        pList->listHeadNodePtr->prevNode = newNode;
        pList->listHeadNodePtr = newNode;
    }

    // setting newNode to be the current item
    pList->currentNodePtr = newNode;
    pList->currentItemPtr = newNode->itemPtr;
    pList->totalItemsInList++;
    return 0;
}

// Return current item and take it out of pList. Make the next item the current
// one. If the current pointer is before the start of the pList, or beyond the
// end of the pList, then do not change the pList and return NULL.
void* List_remove(List* pList) {
    assert(pList != NULL);
    // check if currentItem is already before or beyond the list (currentItemPtr == Null)
    if (pList->currentItemPtr == NULL) {
        // currentNodePtrStatus should already be set so don't change anything
        return NULL;
    }

    void* returnValue = pList->currentItemPtr;
    Node* currentNodeNextPtr = pList->currentNodePtr->nextNode;
    Node* currentNodePrevPtr = pList->currentNodePtr->prevNode;

    // if there is only 1 item in the list
    if (pList->totalItemsInList == 1) {
        pList->listHeadNodePtr = NULL;
        pList->listTailNodePtr = NULL;
        freeNode(pList->currentNodePtr);
        pList->currentNodePtr = NULL;
        pList->currentItemPtr = NULL;
        pList->currentNodePtrStatus = LIST_OOB_START;
    }

    // if the currentNodeptr is at the Head
    else if (pList->currentNodePtr == pList->listHeadNodePtr) {
        pList->listHeadNodePtr = currentNodeNextPtr;
        currentNodeNextPtr->prevNode = NULL;
        freeNode(pList->currentNodePtr);
        pList->currentNodePtr = currentNodeNextPtr;
        pList->currentItemPtr = currentNodeNextPtr->itemPtr;
    }

    // if the currentNodeptr is at the tail
    else if (pList->currentNodePtr == pList->listTailNodePtr) {
        pList->listTailNodePtr = currentNodePrevPtr;
        currentNodePrevPtr->nextNode = NULL;
        freeNode(pList->currentNodePtr);
        pList->currentNodePtr = NULL;
        pList->currentItemPtr = NULL;
        pList->currentNodePtrStatus = LIST_OOB_END;
    }
    // currentNodeptr is valid and is not the head or tail
    else {
        currentNodePrevPtr->nextNode = currentNodeNextPtr;
        currentNodeNextPtr->prevNode = currentNodePrevPtr;
        freeNode(pList->currentNodePtr);
        pList->currentNodePtr = currentNodeNextPtr;
        pList->currentItemPtr = currentNodeNextPtr->itemPtr;
    }

    pList->totalItemsInList--;
    return returnValue;
}

// Return last item and take it out of pList. Make the new last item the current
// one. Return NULL if pList is initially empty.
void* List_trim(List* pList) {
    assert(pList != NULL);

    // check if pList is empty
    if (pList->totalItemsInList == 0) {
        return NULL;
    }

    void* returnValue = pList->listTailNodePtr->itemPtr;

    // if there is only 1 item in the list
    if (pList->totalItemsInList == 1) {
        pList->listHeadNodePtr = NULL;
        pList->listTailNodePtr = NULL;
        freeNode(pList->currentNodePtr);
        pList->currentNodePtr = NULL;
        pList->currentItemPtr = NULL;
        pList->currentNodePtrStatus = LIST_OOB_START;
    } else {
        Node* oldLastNode = pList->listTailNodePtr;

        // Set current ptr to be the new last item
        pList->listTailNodePtr = oldLastNode->prevNode;
        pList->currentNodePtr = pList->listTailNodePtr;
        pList->currentItemPtr = pList->currentNodePtr->itemPtr;

        freeNode(oldLastNode);
    }

    pList->totalItemsInList--;
    return returnValue;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current
// pointer of pList1. pList2 no longer exists after the operation; its head is
// available for future operations.
void List_concat(List* pList1, List* pList2) {
    assert(pList1 != NULL && pList2 != NULL);
    // If there are no items in pList1 then set pList1 to pList2
    if (pList1->totalItemsInList == 0) {
        pList1->listHeadNodePtr = pList2->listHeadNodePtr;
        pList1->listTailNodePtr = pList2->listTailNodePtr;
    }
    //  Only do work if pList2 has items. If pList2 has no items then nothing to concatonate.
    else if (pList2->totalItemsInList != 0) {
        pList1->listTailNodePtr->nextNode = pList2->listHeadNodePtr;
        pList2->listHeadNodePtr->prevNode = pList1->listTailNodePtr;
        pList1->listTailNodePtr = pList2->listTailNodePtr;
    }
    pList1->totalItemsInList = pList1->totalItemsInList + pList2->totalItemsInList;

    // Now free the list head from pList2 so we can re-use it in the future
    freeList(pList2);
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as:
// (*pItemFreeFn)(itemToBeFreedFromNode); pList and all its nodes no longer
// exists after the operation; its head and nodes are available for future
// operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
    assert(pList != NULL);
    // Only free nodes if nodes exists in the list
    if (pList->totalItemsInList != 0) {
        Node* nextNode = pList->listHeadNodePtr->nextNode;
        Node* currentNode = pList->listHeadNodePtr;
        while (nextNode != NULL) {
            // Using the function pointer which will be implemented by the user that free an item
            (*pItemFreeFn)(currentNode->itemPtr);
            freeNode(currentNode);

            currentNode = nextNode;
            nextNode = nextNode->nextNode;
        }

        // When we exit the loop we still have the current node remaining to free
        (*pItemFreeFn)(currentNode->itemPtr);
        freeNode(currentNode);
    }

    // Now free the list head so it can be used in the future
    freeList(pList);
}

// Search pList, starting at the current item, until the end is reached or a
// match is found. In this context, a match is determined by the comparator
// parameter. This parameter is a pointer to a routine that takes as its first
// argument an item pointer, and as its second argument pComparisonArg.
// Comparator returns 0 if the item and comparisonArg don't match, or 1 if they
// do. Exactly what constitutes a match is up to the implementor of comparator.
//
// If a match is found, the current pointer is left at the matched item and the
// pointer to that item is returned. If no match is found, the current pointer
// is left beyond the end of the list and a NULL pointer is returned.
//
// If the current pointer is before the start of the pList, then start searching
// from the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    assert(pList != NULL);
    // check if pList is empty
    if (pList->totalItemsInList == 0) {
        return NULL;
    }

    if (pList->currentItemPtr == NULL) {
        if (pList->currentNodePtrStatus == LIST_OOB_START) {
            // set the current node to the start of the list to start the search
            pList->currentNodePtr = pList->listHeadNodePtr;
            pList->currentItemPtr = pList->currentNodePtr->itemPtr;
        } else {
            // pList->currentNodePtrStatus will be LIST_OOB_END so return Null since we are already beyond the end of the list
            return NULL;
        }
    }
    // While loop to iterate through the current pointer to the end to see if our search ever succeeds
    while (pList->currentNodePtr != NULL) {
        pList->currentItemPtr = pList->currentNodePtr->itemPtr;
        // Using the function pointer which will be implemented by the user
        if (((*pComparator)(pList->currentItemPtr, pComparisonArg)) == 1) {
            // if the current item passes the comparison with given pComparison search completed
            return pList->currentItemPtr;
        }
        pList->currentNodePtr = pList->currentNodePtr->nextNode;
    }

    // If we ever reach this we've reached the end of the list with no match
    // pList->currentNodePtr already has status NULL since we exited the while loop
    pList->currentItemPtr = NULL;  // Need to set it to NULL
    pList->currentNodePtrStatus = LIST_OOB_END;

    return NULL;
}