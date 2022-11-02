#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include "list.h"

static Node nodeList[LIST_MAX_NUM_NODES];
static List headList[LIST_MAX_NUM_HEADS];
static int availableHeads[LIST_MAX_NUM_HEADS];
static int nodeCount = 0, availableHeadTail = 0, firstAllocation = 1;

// Swaps the node with the node at the very top of the list
static int pushToTop(Node *p){
    if (p->nodeIndex == nodeCount-1) {
        return 0;
    }
    Node temp = *p;
    *p = nodeList[nodeCount-1];
    p->nodeIndex = temp.nodeIndex; 
    nodeList[nodeCount-1] = temp;
    nodeList[nodeCount-1].nodeIndex = nodeCount-1;
    return 1;
}

// General Error Handling:
// Client code is assumed never to call these functions with a NULL List pointer, or 
// bad List pointer. If it does, any behaviour is permitted (such as crashing).
// HINT: Use assert(pList != NULL); just to add a nice check, but not required.

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.

List* List_create() {
    // First Allocation - make an array for all available heads.
    if (firstAllocation) {
        for (int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
            // Push all free heads into an array in increasing order.
            availableHeads[i] = LIST_MAX_NUM_HEADS-1-i;
        }
        availableHeadTail = LIST_MAX_NUM_HEADS-1;
        firstAllocation = 0;
    }
    // If there are available heads, set default parameters and return its pointer.
    if (availableHeadTail >= 0 && availableHeadTail < LIST_MAX_NUM_HEADS) {
        int pos = availableHeads[availableHeadTail];
        headList[pos].currentItem = (void*)LIST_OOB_START;
        headList[pos].currentNode = NULL;
        headList[pos].head = NULL;
        headList[pos].tail = NULL;
        headList[pos].length = 0;
        headList[pos].listIndex = pos;
        // Replace the value in the last index (the position of a free head) with -1
        // and decrement the count of the tail.
        availableHeads[availableHeadTail--] = -1;
        
        return &headList[pos];
    } else {
        return NULL;
    }
}

// Returns the number of items in pList.
int List_count(List* pList) {
    assert(pList != NULL);
    return pList->length;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    assert(pList != NULL);
    if (pList->length > 0) {
        pList->currentNode = pList->head;
        pList->currentItem = pList->currentNode->item;
        return pList->currentItem;
    } else {
        pList->currentItem = NULL;
        return NULL;
    }
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    assert(pList != NULL);
    if (pList->length > 0) {
        pList->currentNode = pList->tail;
        pList->currentItem = pList->tail->item;
        return pList->currentItem;
    } else {
        pList->currentItem = NULL;
        return NULL;
    }
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
    assert(pList != NULL);
    if (pList->currentItem == (void*)LIST_OOB_END) {
        return NULL;
    }
    if (pList->currentNode->nextNode) {
        // If it has a next node, return the next node.
        pList->currentNode = pList->currentNode->nextNode;
        pList->currentItem = pList->currentNode->item;
        return pList->currentItem;
    } else {
        // if it doesn't have a next node, it is at the end of the list.
        pList->currentItem = (void*)LIST_OOB_END;
        pList->currentNode = NULL;
        return NULL;
    }
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
    assert(pList != NULL);
    if (pList->currentItem == (void*)LIST_OOB_START) {
        return NULL;
    }
    if (pList->currentNode->prevNode) {
        // If it has a previous node, return the previous node.
        pList->currentNode = pList->currentNode->prevNode;
        pList->currentItem = pList->currentNode->item;
        return pList->currentItem;
    } else {
        // if it doesn't have a previous node, it is at the end of the list.
        pList->currentItem = (void*)LIST_OOB_START;
        pList->currentNode = NULL;
        return NULL;
    }
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
    assert(pList != NULL);
    return pList->currentItem;
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem) {
    assert(pList != NULL);

    if (nodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }
    // Initialize a Node
    nodeCount++;
    pList->length++;
    nodeList[nodeCount-1].item = pItem;
    nodeList[nodeCount-1].nodeIndex = nodeCount-1;
    nodeList[nodeCount-1].nextNode = NULL;
    nodeList[nodeCount-1].prevNode = NULL;

    if (pList->length == 1) {
        // If its the first node, skip the rest and just insert it.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        pList->head = &nodeList[nodeCount-1];
        pList->tail = &nodeList[nodeCount-1];
        return LIST_SUCCESS;
    }

    if (pList->currentItem == (void*)LIST_OOB_START) {
        // If current pointer is before the start:
        // Set the new Node as the head.
        pList->head->prevNode = &nodeList[nodeCount-1];
        nodeList[nodeCount-1].nextNode = pList->head;
        pList->head = &nodeList[nodeCount-1];
        // Replace current Node.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        return LIST_SUCCESS;

    } else if (pList->currentItem == (void*)LIST_OOB_END) {
        // If current pointer is after the end:
        // Set the new Node as the tail.
        pList->tail->nextNode = &nodeList[nodeCount-1];
       nodeList[nodeCount-1].prevNode = pList->tail;
        pList->tail = &nodeList[nodeCount-1];
        // Replace current Node.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        return LIST_SUCCESS;

    } else {
        // Replace current Node and Item with the new Node.
        if (pList->currentNode->nextNode) {
            // If a node after the current node exists, set it's previous node to be the new node.
            nodeList[nodeCount-1].nextNode = pList->currentNode->nextNode;
            nodeList[nodeCount-1].nextNode->prevNode = &nodeList[nodeCount-1];
        } else {
            // Set the tail of the list to be the new Node.
            pList->tail = &nodeList[nodeCount-1];
        }
        pList->currentNode->nextNode = &nodeList[nodeCount-1];
        nodeList[nodeCount-1].prevNode = pList->currentNode;
        // Replace current Node.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        return LIST_SUCCESS;
    }
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem) {
    assert(pList != NULL);

    if (nodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }
    // Initialize a Node
    nodeCount++;
    pList->length++;
    nodeList[nodeCount-1].item = pItem;
    nodeList[nodeCount-1].nodeIndex = nodeCount-1;
    nodeList[nodeCount-1].nextNode = NULL;
    nodeList[nodeCount-1].prevNode = NULL;

    if (pList->length == 1) {
        // If its the first node, skip the rest and just insert it.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        pList->head = &nodeList[nodeCount-1];
        pList->tail = &nodeList[nodeCount-1];
        return LIST_SUCCESS;
    }

    if (pList->currentItem == (void*)LIST_OOB_START) {
        // If current pointer is before the start:
        // Set the new Node as the head.
        pList->head->prevNode = &nodeList[nodeCount-1];
        nodeList[nodeCount-1].nextNode = pList->head;
        pList->head = &nodeList[nodeCount-1];
        // Replace current Node.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        return LIST_SUCCESS;

    } else if (pList->currentItem == (void*)LIST_OOB_END) {
        // If current pointer is after the end:
        // Set the new Node as the tail.
        pList->tail->nextNode = &nodeList[nodeCount-1];
       nodeList[nodeCount-1].prevNode = pList->tail;
        pList->tail = &nodeList[nodeCount-1];
        // Replace current Node.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        return LIST_SUCCESS;

    } else {
        // Replace current Node and Item with the new Node.
        if (pList->currentNode->prevNode) {
            // If a node before the current node exists, set it's next node to be the new node.
            nodeList[nodeCount-1].prevNode = pList->currentNode->prevNode;
            nodeList[nodeCount-1].prevNode->nextNode = &nodeList[nodeCount-1];
        } else {
            // Set the head of the list to be the new Node.
            pList->head = &nodeList[nodeCount-1];
        }
        pList->currentNode->prevNode = &nodeList[nodeCount-1];
        nodeList[nodeCount-1].nextNode = pList->currentNode;
        // Replace current Node.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        return LIST_SUCCESS;
    }
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
    assert(pList != NULL);

    if (nodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }
    // Initialize a Node
    nodeCount++;
    pList->length++;
    nodeList[nodeCount-1].item = pItem;
    nodeList[nodeCount-1].nodeIndex = nodeCount-1;
    nodeList[nodeCount-1].nextNode = NULL;
    nodeList[nodeCount-1].prevNode = NULL;

    if (pList->length == 1) {
        // If its the first node, skip the rest and just insert it.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        pList->head = &nodeList[nodeCount-1];
        pList->tail = &nodeList[nodeCount-1];
        return LIST_SUCCESS;
    }
    // If current pointer is after the end:
    // Set the new Node as the tail.
    pList->tail->nextNode = &nodeList[nodeCount-1];
    nodeList[nodeCount-1].prevNode = pList->tail;
    pList->tail = &nodeList[nodeCount-1];
    // Replace current Node.
    pList->currentNode = &nodeList[nodeCount-1];
    pList->currentItem = nodeList[nodeCount-1].item;
    return LIST_SUCCESS;
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
    assert(pList != NULL);

    if (nodeCount >= LIST_MAX_NUM_NODES) {
        return LIST_FAIL;
    }
    // Initialize a Node
    nodeCount++;
    pList->length++;
    nodeList[nodeCount-1].item = pItem;
    nodeList[nodeCount-1].nodeIndex = nodeCount-1;
    nodeList[nodeCount-1].nextNode = NULL;
    nodeList[nodeCount-1].prevNode = NULL;
    
    if (pList->length == 1) {
        // If its the first node, skip the rest and just insert it.
        pList->currentNode = &nodeList[nodeCount-1];
        pList->currentItem = nodeList[nodeCount-1].item;
        pList->head = &nodeList[nodeCount-1];
        pList->tail = &nodeList[nodeCount-1];
        return LIST_SUCCESS;
    }
    // If current pointer is before the start:
    // Set the new Node as the head.
    pList->head->prevNode = &nodeList[nodeCount-1];
    nodeList[nodeCount-1].nextNode = pList->head;
    pList->head = &nodeList[nodeCount-1];
    // Replace current Node.
    pList->currentNode = &nodeList[nodeCount-1];
    pList->currentItem = nodeList[nodeCount-1].item;
    return LIST_SUCCESS;
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
    assert(pList != NULL);
    if (pList->currentItem == (void*)LIST_OOB_START || pList->currentItem == (void*)LIST_OOB_END) {
        return NULL;
    } else if (pList->length == 1) {
        // If its the last node, skip the rest and delete it after resetting parameters.
        nodeList[nodeCount-1].item = NULL;
        nodeList[nodeCount-1].nodeIndex = 0;
        nodeList[nodeCount-1].nextNode = NULL;
        nodeList[nodeCount-1].prevNode = NULL;
        nodeCount--;
        pList->length--;
        pList->head = NULL;
        pList->tail = NULL;
        pList->currentNode = NULL;
        pList->currentItem = (void*)LIST_OOB_START;
        return pList->currentItem;
    }

    int removedNodeIsTail = 0;
    void* res = pList->currentItem;
    int tempIndex = pList->currentNode->nodeIndex;
    Node* nextCurr = pList->currentNode->nextNode;

    // For when the next node is the at the last position.
    if (nextCurr == &nodeList[nodeCount-1]) {
        nextCurr = pList->currentNode;
    }
    // Check if node that was swapped is a tail/head; correct links.
    if (pList->currentNode == pList->head) {
        pList->currentNode->nextNode->prevNode = NULL;
        pList->head = nextCurr;
    } else if (pList->currentNode == pList->tail) {
        pList->currentNode->prevNode->nextNode = NULL;
        pList->tail = pList->currentNode->prevNode;
        //if the node removed is the tail, set the current node to be the node before.
        removedNodeIsTail = 1;
    } else {
        // 1-2-3 => 1-3 (when 2 is current node)
        pList->currentNode->prevNode->nextNode = pList->currentNode->nextNode;
        pList->currentNode->nextNode->prevNode = pList->currentNode->prevNode;
    }

    if (pushToTop(pList->currentNode)) {
        // Swap places in the array with the newest node to able to reuse the memory.
        // If the current node is not already at the top of the array,
        // check if node that was swapped is a tail/head; correct links.
        if (nodeList[tempIndex].prevNode == NULL) {
            // if the swapped node is a head.
            pList->head = &nodeList[tempIndex];
            if (pList->length > 2) {
                // If the list has more than two nodes, then set it's links properly.
                pList->head->nextNode->prevNode = pList->head;
            } else {
                // If the list has two nodes, set the new current node to the same position.
                nextCurr = pList->head;
            }
            
        } else if (nodeList[tempIndex].nextNode == NULL) {
            // If the swapped node is a tail.
            pList->tail = &nodeList[tempIndex];
            if (pList->length > 2) {
                pList->tail->prevNode->nextNode = pList->tail;
            } 

        } else {
            // if its neither, correct links normally.
            nodeList[tempIndex].prevNode->nextNode = &nodeList[tempIndex];
            nodeList[tempIndex].nextNode->prevNode = &nodeList[tempIndex];
        }
    }

    // Set next node to be current node.
    if (removedNodeIsTail) {
        // if the node removed was a tail, set it to be at the end of the list.
        pList->currentNode = NULL;
        pList->currentItem = (void*)LIST_OOB_END;
    } else {
        pList->currentNode = nextCurr;
        pList->currentItem = nextCurr->item;
    }
    
    // Lower total node count and length of list.
    // Clean the deleted node.
    nodeList[nodeCount-1].item = ((void*)0);
    nodeList[nodeCount-1].nodeIndex = 0;
    nodeList[nodeCount-1].nextNode = NULL;
    nodeList[nodeCount-1].prevNode = NULL;
    nodeCount--;
    pList->length--;
    return res;
}


// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
    if (pList->length <= 0) {
        return NULL;

    } else if (pList->length == 1) {
        // If its the last node, skip the rest and delete it after resetting parameters.
        nodeList[nodeCount-1].item = NULL;
        nodeList[nodeCount-1].nodeIndex = 0;
        nodeList[nodeCount-1].nextNode = NULL;
        nodeList[nodeCount-1].prevNode = NULL;
        nodeCount--;
        pList->length--;
        pList->head = NULL;
        pList->tail = NULL;
        pList->currentNode = NULL;
        pList->currentItem = (void*)LIST_OOB_START;
        return pList->currentItem;
    }
    void* res = pList->tail->item;
    // Set the current pointer to the last node and remove it.
    List_last(pList);
    List_remove(pList);
    // Set the current pointer to the new last node.
    pList->tail->nextNode = NULL;
    pList->currentNode = pList->tail;
    pList->currentItem = pList->tail->item;
    return res;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2) {
    assert(pList1 != NULL);
    assert(pList2 != NULL);
    int index = pList2->listIndex;
    
    // Correct links to the head and tail.
    if (pList1->tail) {
        pList1->tail->nextNode = pList2->head;
        pList2->head->prevNode = pList1->tail;
    }
    pList1->tail = pList2->tail;
    pList1->length += pList2->length;

    // Free the list and return the list index to the array of available heads.
    headList[index].currentNode = NULL;
    headList[index].currentItem = NULL;
    headList[index].head = NULL;
    headList[index].tail = NULL;
    headList[index].length = 0;
    headList[index].listIndex = 0;
    availableHeads[++availableHeadTail] = index;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
    int index = pList->listIndex;

    //Removing all Nodes
    while (pList->length > 0) {
        (*pItemFreeFn)(pList->tail->item);
        List_trim(pList);
    }
    
    // Free the list and return the list index to the array of available heads.
    (*pItemFreeFn)(headList[index].currentItem);
    headList[index].currentNode = NULL;
    headList[index].head = NULL;
    headList[index].tail = NULL;
    headList[index].length = 0;
    headList[index].listIndex = 0;
    availableHeads[++availableHeadTail] = index;

    nodeCount -= pList->length;
}

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
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    if (pList->currentItem == (void*)LIST_OOB_START) {
        // If the current pointer is at the start of the list, start at the first node if possible.
        if (!List_first(pList)) {
            return NULL;
        } 
    }
    Node* curr = pList->currentNode;

    while (curr) {
        // Go through the list to find the node corresponding to the comparator.
        if((*pComparator)(curr->item, &pComparisonArg)) {
            // If found, set it as the current node and return it.
            pList->currentNode = curr;
            pList->currentItem = curr->item;
            return pList->currentItem;
        }   
        curr = curr->nextNode;
    }
    // If not found, set current pointer to null.
    pList->currentNode = NULL;
    pList->currentItem = (void*)LIST_OOB_END;
    return NULL;
}


