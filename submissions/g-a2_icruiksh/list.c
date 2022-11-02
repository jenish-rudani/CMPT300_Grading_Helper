
#include "list.h"


// All functions must be O(1) unless otherwise stated
static Node nodes[LIST_MAX_NUM_NODES];
static List heads[LIST_MAX_NUM_HEADS];


// STACK FUNCTIONS
// Use a stack (static array with moving top index) to make finding the next free node/head happen in O(1) time
static Node* freeNodesStack[LIST_MAX_NUM_NODES];
static List* freeHeadsStack[LIST_MAX_NUM_HEADS];

static int freeNodesStackIndex = 0;
static int freeHeadsStackIndex = 0;

static bool stacksInitialized = false;

// O(n)
static void initializeHeadNodeStacks() {
    for(int i = 0; i < LIST_MAX_NUM_NODES; i++) {
        nodes[i].item = NULL;
        nodes[i].nextNode = NULL;
        nodes[i].prevNode = NULL;
        freeNodesStack[i] = &(nodes[i]);
    }

    freeNodesStackIndex = LIST_MAX_NUM_NODES - 1;

    for(int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
        heads[i].currentNode = NULL;
        heads[i].headNode = NULL;
        heads[i].tailNode = NULL;
        heads[i].size = 0;
        freeHeadsStack[i] = &(heads[i]);
    }

    freeHeadsStackIndex = LIST_MAX_NUM_HEADS - 1;

    // Both stacks are now full

    stacksInitialized = true;
}

static Node* popNextFreeNode() {
    if(freeNodesStackIndex == -1) {
        return NULL;
    }

    return freeNodesStack[freeNodesStackIndex--];
}

static void pushFreeNode(Node* freeNode) {
    if(freeNodesStackIndex < LIST_MAX_NUM_NODES - 1) {
        freeNodesStack[++freeNodesStackIndex] = freeNode;
    }
}

static List* popNextFreeHead() {
    if(freeHeadsStackIndex == -1) {
        return NULL;
    }

    return freeHeadsStack[freeHeadsStackIndex--];
}

static void pushFreeHead(List* freeHead) {
    if(freeHeadsStackIndex < LIST_MAX_NUM_HEADS - 1) {
        freeHeadsStack[++freeHeadsStackIndex] = freeHead;
    }
}

static bool headIsOnStack(List* potentialFreeHead) {
    for(int i = freeHeadsStackIndex; i >= 0; i--) {
        if(potentialFreeHead == freeHeadsStack[i]) {
            return true;
        }
    }

    return false;
}
// END OF STACK FUNCTIONS


// May be O(n)
List* List_create() {
    if(!stacksInitialized) {
        initializeHeadNodeStacks(); // O(n), only runs once though
    }

    return popNextFreeHead();
}


int List_count(List* pList) {
    if(pList == NULL) {
        return LIST_FAIL;
    }

    return pList->size;
}


void* List_first(List* pList) {
    if(pList == NULL) {
        return NULL;
    }

    if(pList->size == 0) {
        return NULL;
    }

    pList->currentNode = pList->headNode;

    return pList->headNode->item;
}


void* List_last(List* pList) {
    if(pList == NULL) {
        return NULL;
    }

    if(pList->size == 0) {
        return NULL;
    }

    pList->currentNode = pList->tailNode;

    return pList->tailNode->item;
}


void* List_next(List* pList) {
    if(pList == NULL) {
        return NULL;
    }

    // Checks empty and OOB cases
    if(pList->currentNode == NULL) {
        return NULL;
    }

    pList->currentNode = pList->currentNode->nextNode;

    if(pList->currentNode == NULL) {
        return NULL;
    }

    return pList->currentNode->item;
}


void* List_prev(List* pList) {
    if(pList == NULL) {
        return NULL;
    }

    // Checks empty and OOB cases
    if(pList->currentNode == NULL) {
        return NULL;
    }

    pList->currentNode = pList->currentNode->prevNode;

    if(pList->currentNode == NULL) {
        return NULL;
    }
    return pList->currentNode->item;
}


void* List_curr(List* pList) {
    if(pList == NULL) {
        return NULL;
    }

    // Checks empty and OOB cases
    if(pList->currentNode == NULL) {
        return NULL;
    }

    return pList->currentNode->item;
}


int List_insert_after(List* pList, void* pItem) {
    if(pList == NULL) {
        return LIST_FAIL;
    }

    // Check OOB case
    if(pList->currentNode == NULL) {
        return LIST_FAIL;
    }
    
    Node* pNode = popNextFreeNode();

    if(pNode == NULL) {
        return LIST_FAIL;
    }

    pNode->item = pItem;

    // Empty case
    if(pList->size == 0) {
        pNode->nextNode = NULL;
        pNode->prevNode = NULL;

        pList->headNode = pNode;
        pList->tailNode = pNode;
    }
    // Edge case
    else if(pList->currentNode == pList->tailNode) {
        pList->tailNode->nextNode = pNode;

        pNode->prevNode = pList->tailNode;
        pNode->nextNode = NULL;

        pList->tailNode = pNode;
    }
    // General case
    else {
        pNode->prevNode = pList->currentNode;
        pNode->nextNode = pList->currentNode->nextNode;

        pNode->prevNode->nextNode = pNode;
        pNode->nextNode->prevNode = pNode;
    }

    pList->currentNode = pNode;

    pList->size += 1;

    return LIST_SUCCESS;
}


int List_insert_before(List* pList, void* pItem) {
    if(pList == NULL) {
        return LIST_FAIL;
    }

    // Check OOB case
    if(pList->currentNode == NULL) {
        return LIST_FAIL;
    }
    
    Node* pNode = popNextFreeNode();

    if(pNode == NULL) {
        return LIST_FAIL;
    }

    pNode->item = pItem;

    // Empty case
    if(pList->headNode == NULL) {
        pNode->nextNode = NULL;
        pNode->prevNode = NULL;

        pList->headNode = pNode;
        pList->tailNode = pNode;
    }
    // Edge case
    else if(pList->currentNode == pList->headNode) {
        pList->headNode->prevNode = pNode;

        pNode->nextNode = pList->headNode;
        pNode->prevNode = NULL;

        pList->headNode = pNode;
    }
    // General case
    else {
        pNode->nextNode = pList->currentNode;
        pNode->prevNode = pList->currentNode->prevNode;

        pNode->prevNode->nextNode = pNode;
        pNode->nextNode->prevNode = pNode;
    }

    pList->currentNode = pNode;

    pList->size += 1;

    return LIST_SUCCESS;
}


int List_append(List* pList, void* pItem) {
    if(pList == NULL) {
        return LIST_FAIL;
    }
    
    Node* pNode = popNextFreeNode();

    if(pNode == NULL) {
        return LIST_FAIL;
    }

    pNode->item = pItem;

    pNode->prevNode = pList->tailNode;
    pNode->nextNode = NULL;

    // Check for emptiness
    if(pList->headNode != NULL) {
        pNode->prevNode->nextNode = pNode;
    }
    else {
        // If empty is the only time append modifies the head
        pList->headNode = pNode;
    }

    pList->currentNode = pNode;
    pList->tailNode = pNode;

    pList->size += 1;

    return LIST_SUCCESS;
}


int List_prepend(List* pList, void* pItem) {
    if(pList == NULL) {
        return LIST_FAIL;
    }
    
    Node* pNode = popNextFreeNode();

    if(pNode == NULL) {
        return LIST_FAIL;
    }

    pNode->item = pItem;

    pNode->nextNode = pList->headNode;
    pNode->prevNode = NULL;

    // Check for emptiness
    if(pList->headNode != NULL) {
        pNode->nextNode->prevNode = pNode;
    }
    else {
        // If empty is the only time prepend modifies the tail
        pList->tailNode = pNode;
    }

    pList->currentNode = pNode;
    pList->headNode = pNode;

    pList->size += 1;

    return LIST_SUCCESS;
}


void* List_remove(List* pList) {
    if(pList == NULL) {
        return NULL;
    }

    // Check empty case
    if(pList->size == 0) {
        return NULL;
    }

    // Check OOB case
    if(pList->currentNode == NULL) {
        return NULL;
    }

    Node* removedNode = pList->currentNode;
    void* removedItem = removedNode->item;

    // Size 1 case
    if(pList->headNode == pList->tailNode) {
        pList->headNode = NULL;
        pList->tailNode = NULL;
        pList->currentNode = NULL;
    }
    // Head case
    else if(pList->currentNode == pList->headNode) {
        pList->headNode = pList->headNode->nextNode;
        pList->headNode->prevNode = NULL;

        pList->currentNode = pList->headNode;
    }
    // Tail case
    else if(pList->currentNode == pList->tailNode) {
        pList->tailNode = pList->tailNode->prevNode;
        pList->tailNode->nextNode = NULL;

        pList->currentNode = pList->tailNode;
    }
    // General Case
    else {
        pList->currentNode->nextNode->prevNode = pList->currentNode->prevNode;
        pList->currentNode->prevNode->nextNode = pList->currentNode->nextNode;

        pList->currentNode = pList->currentNode->nextNode;
    }

    removedNode->nextNode = NULL;
    removedNode->prevNode = NULL;
    removedNode->item = NULL;

    pushFreeNode(removedNode);

    pList->size -= 1;

    return removedItem;
}


void* List_trim(List* pList) {
    pList->currentNode = pList->tailNode;
    return List_remove(pList);
}


void List_concat(List* pList1, List* pList2) {
    // If both lists are not empty
    if(pList1->headNode != NULL && pList2->headNode != NULL) {
        pList1->tailNode->nextNode = pList2->headNode;
        pList2->headNode->prevNode = pList1->tailNode;
    }
    // Just pList1 is empty
    else if(pList1->headNode == NULL) {
        pList1->headNode = pList2->headNode;
    }
    // Nothing if just pList2 is empty

    pList1->size += pList2->size;

    if(pList2->tailNode != NULL) {
        // Set list 1 tail. No need to set it if list 2 is empty
        pList1->tailNode = pList2->tailNode;
    }

    pList2->headNode = NULL;
    pList2->tailNode = NULL;
    pList2->currentNode = NULL;
    pList2->size = 0;

    if(!headIsOnStack(pList2)) {
        pushFreeHead(pList2);
    }
}

// May be O(n)
void List_free(List* pList, FREE_FN pItemFreeFn) {

    if(pList == NULL) {
        return;
    }

    while(List_count(pList) != 0) {
        void* removedItem = List_trim(pList);
        if(pItemFreeFn != NULL)
            (*pItemFreeFn)(removedItem);
    }

    pList->headNode = NULL;
    pList->tailNode = NULL;
    pList->currentNode = NULL;

    if(!headIsOnStack(pList)) {
        pushFreeHead(pList);
    }
}

// May be O(n)
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    if(pComparator == NULL || pComparisonArg == NULL) {
        return NULL;
    }

    if(pList->size == 0) {
        return NULL;
    }

    if(pList->currentNode == NULL) {
        pList->currentNode = pList->headNode;
    }

    for(pList->currentNode += 0;
            pList->currentNode != NULL;
                pList->currentNode = pList->currentNode->nextNode)
    {
        if((*pComparator)(pList->currentNode->item, pComparisonArg)) {
            return pList->currentNode->item;
        }
    }

    return NULL;
}