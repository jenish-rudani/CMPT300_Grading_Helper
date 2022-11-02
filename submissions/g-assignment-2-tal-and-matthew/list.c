#include <stdio.h>

#include "list.h"

// Pre-allocate space for heads
static List listHeads[LIST_MAX_NUM_HEADS];
static int listHeadCount = 0;

// Pre-allocate nodes for keeping a list of available heads
static Node availableHeadsNodes[LIST_MAX_NUM_HEADS];
static List availableHeads;

// Pre-allocate space for nodes
static Node listNodes[LIST_MAX_NUM_NODES];
static int listNodeCount = 0;

static bool initialized = false;

// Creates a list for use in keeping track of what lists are being used or not
void initializeAvailableHeads() {
    Node firstInitilizationNode = {.listNodePosition = 0, .leftNode = NULL, .rightNode = NULL};
    availableHeads.listLength++;

    availableHeads.currentNode = &availableHeadsNodes[0];
    availableHeads.firstNode = &availableHeadsNodes[0];
    availableHeads.lastNode = &availableHeadsNodes[0];

    for (int i = 1; i < LIST_MAX_NUM_HEADS; i++) {
        Node initilizationNode = {.listNodePosition = i, .leftNode = availableHeads.lastNode, .rightNode = NULL};
        availableHeadsNodes[i] = initilizationNode;
        availableHeads.lastNode = &availableHeadsNodes[i];
        availableHeads.listLength++;
    }
}

// Remove element from queue
// Used when adding a list/node
int dequeue() {
    int removedValue = availableHeads.lastNode->listNodePosition;
    availableHeads.listLength--;

    if (availableHeads.listLength == 0) {
    } else {
        availableHeads.lastNode = availableHeads.lastNode->leftNode;
    }

    return removedValue;
}

// Add element to end of queue
// Used when removing a list/node
void queue(int addedValue) {
    if (List_count(&availableHeads) != 0) {
        availableHeadsNodes[addedValue].leftNode = availableHeads.lastNode;
    }   
    
    availableHeads.lastNode = &availableHeadsNodes[addedValue];
    availableHeads.listLength++; 
}

// When a node is removed it takes the last node in the node array and moves it into the new empty position
void updateNodePosition(int newPosition) {
    if (listNodeCount == 1) {
        listNodeCount--;
        return;
    }


    // Save the nodes that the last node point to
    Node* leftMovedNode = listNodes[listNodeCount].leftNode;
    Node* rightMovedNode = listNodes[listNodeCount].rightNode;

    // Move the last node to the new position
    listNodes[newPosition] = listNodes[listNodeCount];

    // Update the pointers that point to the new node
    if (leftMovedNode != NULL) {
        leftMovedNode->rightNode = &listNodes[newPosition];
    }

    if (rightMovedNode != NULL) {
        rightMovedNode->leftNode = &listNodes[newPosition];
    }

    listNodeCount--;
}

// Function adds a new node to a list that is currently empty
void addToEmpty(List* pList, void* pItem) {

    // Create new node and add it to the node array
    Node newNode = {.listNodePosition = listNodeCount, .data = pItem, .leftNode = NULL, .rightNode = NULL};
    listNodeCount++;

    listNodes[newNode.listNodePosition] = newNode;

    // Set nodes in array to 1, and set the current/first/last node to the new node
    pList->listLength = List_count(pList) + 1;
    pList->currentNode = &listNodes[newNode.listNodePosition];
    pList->firstNode = &listNodes[newNode.listNodePosition];
    pList->lastNode = &listNodes[newNode.listNodePosition];
}

// Adds a node in between two other nodes, if either nodes don't exist then it will stitch the node to one side
void addNodeBetween(List* pList, Node* leftStitchNode, Node* rightStitchNode, void* pItem) {

    Node newNode = {.listNodePosition = listNodeCount, .data = pItem, .leftNode = NULL, .rightNode = NULL};
    listNodeCount++;
    
    listNodes[newNode.listNodePosition] = newNode;
    
    if (leftStitchNode == NULL) {
        // No left node
        listNodes[newNode.listNodePosition].rightNode = rightStitchNode;
        rightStitchNode->leftNode = &listNodes[newNode.listNodePosition];
        pList->firstNode = &listNodes[newNode.listNodePosition];
    } else if (rightStitchNode == NULL) {
        // No right node
        listNodes[newNode.listNodePosition].leftNode = leftStitchNode;
        leftStitchNode->rightNode = &listNodes[newNode.listNodePosition];
        pList->lastNode = &listNodes[newNode.listNodePosition];
    } else {
        // If both nodes exist then stitch new node to the middle
        rightStitchNode->leftNode = &listNodes[newNode.listNodePosition];
        leftStitchNode->rightNode = &listNodes[newNode.listNodePosition];

        listNodes[newNode.listNodePosition].leftNode = leftStitchNode;
        listNodes[newNode.listNodePosition].rightNode = rightStitchNode;
    }

    pList->listLength++;
    pList->currentNode = &listNodes[newNode.listNodePosition];
}

// Adds a node if the current selection is out of bounds, in the direction of which way the list is out of bounds in
void addNodeOutOfBounds(List* pList, void*pItem) {
    if (pList->outOfBoundsDirection == LIST_OOB_START) {
        addNodeBetween(pList, NULL, pList->firstNode, pItem);
    } else {
        addNodeBetween(pList, pList->lastNode, NULL, pItem);
    }

    pList->outOfBounds = false;
}

List* List_create() {
    // Initialization code runs once when a list is first created and sets up system for keeping track of available heads and nodes
    if (initialized == false) {
        initializeAvailableHeads();
        initialized = true;
    }

    // Fails if all arrays are in use
    if (listHeadCount >= LIST_MAX_NUM_HEADS) {
        return NULL;
    }

    List newList = {.listHeadsPosition = dequeue(), .listLength = 0, .outOfBounds = false, .outOfBoundsDirection = LIST_OOB_START, .currentNode = NULL, .firstNode = NULL, .lastNode = NULL};
    listHeadCount++;

    listHeads[newList.listHeadsPosition] = newList;

    return &listHeads[newList.listHeadsPosition];
}

int List_count(List* pList) {
    return pList->listLength;
}

void* List_first(List* pList) {
    if (pList->currentNode == NULL) {
        return NULL;
    }

    pList->outOfBounds = false;
    pList->currentNode = pList->firstNode;
    return pList->firstNode->data;
}

void* List_last(List* pList) {
    if (pList->currentNode == NULL) {
        return NULL;
    }
    
    pList->outOfBounds = false;
    pList->currentNode = pList->lastNode;
    return pList->lastNode->data;
}

void* List_next(List* pList) {
    if (pList->currentNode == NULL) {
        // Return NULL if the current node is NULL
        return NULL;
    } else if (pList->currentNode->rightNode == NULL) {
        // If there is no node to the right then set out of bounds
        pList->outOfBounds = true;
        pList->outOfBoundsDirection = LIST_OOB_END;
        pList->currentNode = NULL;
        return NULL;
    }

    pList->currentNode = pList->currentNode->rightNode;
    return pList->currentNode->data;
}

void* List_prev(List* pList) {
    // Check if you would be going out of bounds/already out of bounds
    if (pList->currentNode == NULL) {
        // Return NULL if the current node is NULL
        return NULL;
    } else if (pList->currentNode->leftNode == NULL) {
        // If there is no node to the left then set out of bounds
        pList->outOfBounds = true;
        pList->outOfBoundsDirection = LIST_OOB_START;
        pList->currentNode = NULL;
        return NULL;
    }
    
    pList->currentNode = pList->currentNode->leftNode;
    return pList->currentNode->data;
}

void* List_curr(List* pList) {
    // If no node is currently selected then return NULL
    if (pList->currentNode == NULL) {
        return NULL;
    }
    
    return pList->currentNode->data;
}

int List_insert_after(List* pList, void* pItem) {
    // Fail if all nodes are in use
    if (listNodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }
    
    if (List_count(pList) == 0) {
        // Check for empty List
        addToEmpty(pList, pItem);
    } else if (pList->outOfBounds == true) {
        // Check for out of bounds
        addNodeOutOfBounds(pList, pItem);

        pList->outOfBounds = false;
    } else {
        addNodeBetween(pList, pList->currentNode, pList->currentNode->rightNode, pItem);
    }

    return LIST_SUCCESS;
}

int List_insert_before(List* pList, void* pItem) {
    // Fail if all nodes are in use
    if (listNodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }
    
    if (List_count(pList) == 0) {
        // Check for empty list
        addToEmpty(pList, pItem);
    } else if (pList->outOfBounds == true) {
        // Check for out of bounds
        addNodeOutOfBounds(pList, pItem);
    } else {
        addNodeBetween(pList, pList->currentNode->leftNode, pList->currentNode, pItem);
    }

    return LIST_SUCCESS;
}

int List_append(List* pList, void* pItem) {
    // Fail if all nodes are in use
    if (listNodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }

    if (List_count(pList) == 0) {
        addToEmpty(pList, pItem);
    } else {
        addNodeBetween(pList, pList->lastNode, NULL, pItem);
    }
    return LIST_SUCCESS;
}

int List_prepend(List* pList, void* pItem) {
    // Fail if all nodes are in use
    if (listNodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }

    if (List_count(pList) == 0) {
        addToEmpty(pList, pItem);
    } else {
        addNodeBetween(pList, NULL, pList->firstNode, pItem);
    }
    return LIST_SUCCESS;
}

void* List_remove(List* pList) {
    // Do nothing if selection is out of bounds or there are no nodes in the list
    if (pList->outOfBounds == true || pList->listLength == 0) {
        return NULL;
    }

    void* removedData = pList->currentNode->data;

    int removedPosition = pList->currentNode->listNodePosition;

    if (pList->listLength == 1) {
        // Remove the last node of a list
        pList->currentNode = NULL;
        pList->firstNode = NULL;
        pList->lastNode = NULL;
    } else if (pList->currentNode->leftNode == NULL) {
        // Remove from beginning of list
        pList->currentNode = pList->currentNode->rightNode;
        pList->firstNode = pList->currentNode;
        pList->currentNode->leftNode = NULL;
    } else if (pList->currentNode->rightNode == NULL) {
        // Remove from past the end of a list
        pList->outOfBounds = true;
        pList->outOfBoundsDirection = LIST_OOB_END;
        pList->lastNode = pList->lastNode->leftNode;
        pList->currentNode = NULL;
    } else {
        // Remove from middle of list
        pList->currentNode = pList->currentNode->leftNode;
        pList->currentNode->rightNode = pList->currentNode->rightNode->rightNode;
        pList->currentNode = pList->currentNode->rightNode;
        pList->currentNode->leftNode = pList->currentNode->leftNode->leftNode;
    }

    updateNodePosition(removedPosition);

    pList->listLength--; 

    return removedData; 
}

void* List_trim(List* pList) {
    // Do nothing if list is empty
    if (pList->listLength == 0) {
        return NULL;
    }
    
    void* removedData = pList->lastNode->data;

    int removedPosition = pList->lastNode->listNodePosition;

    if (pList->listLength == 1) {
        // Trimming last node of a list
        pList->currentNode = NULL;
        pList->firstNode = NULL;
        pList->lastNode = NULL;
    } else {
        pList->lastNode = pList->lastNode->leftNode;
        pList->currentNode = pList->lastNode;
    }

    updateNodePosition(removedPosition);

    pList->listLength--;

    return removedData;
}

void List_concat(List* pList1, List* pList2) {
    // Check if either list is empty when concatinating them
    if (pList1->listLength == 0 && pList2->listLength != 0) {
        // first list is empty
        pList1->firstNode = pList2->firstNode;
        pList1->currentNode = pList1->firstNode;
        pList1->lastNode = pList2->lastNode;
    } else if (pList1->listLength != 0 && pList2->listLength != 0) {
        // both lists are not empty
        pList1->lastNode->rightNode = pList2->firstNode;
        pList2->firstNode->leftNode = pList1->lastNode;
        pList1->lastNode = pList2->lastNode;
        pList1->currentNode = pList1->firstNode;
    }

    listHeadCount--;
    queue(pList2->listHeadsPosition);
}

void List_free(List* pList, FREE_FN pItemFreeFn) {
    const int listSize = List_count(pList);
    for (int i = 0; i < listSize; i++) {
        (*pItemFreeFn)(List_last(pList));
        List_trim(pList);
    }

    listHeadCount--;
    queue(pList->listHeadsPosition);
}

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    // If selection is before list then set selection to the first node
    if (pList->outOfBounds == true && pList->outOfBoundsDirection == LIST_OOB_START) {
        pList->currentNode = pList->firstNode;
        pList->outOfBounds = false;
    }

    // If selection is after list then there is nothing to search through and it returns NULL
    if (pList->outOfBounds == true && pList->outOfBoundsDirection == LIST_OOB_END) {
        return NULL;
    }

    // Search through every node of the list
    while (List_curr(pList) != NULL) {
        // Check if there is a match and return it
        if ((*pComparator)(List_curr(pList), pComparisonArg) == true) {
            return List_curr(pList);
        }

        List_next(pList);
    }
}