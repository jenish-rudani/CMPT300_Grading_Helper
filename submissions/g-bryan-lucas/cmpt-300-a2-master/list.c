#include "list.h"
#include <stdio.h>

static List lists[LIST_MAX_NUM_HEADS] = {};
static int listsIdx[LIST_MAX_NUM_HEADS] = {};
static int listsIdxHead = 0;

static Node nodes[LIST_MAX_NUM_NODES] = {};
static int nodesIdx[LIST_MAX_NUM_NODES] = {};
static int nodesIdxHead = 0;

static bool firstRun = true;

List* List_create() {
    if (listsIdxHead >= LIST_MAX_NUM_HEADS) return NULL;

    if (firstRun) {
        for (int i = 0; i < LIST_MAX_NUM_HEADS; i++)
            listsIdx[i] = i;
        for (int j = 0; j < LIST_MAX_NUM_NODES; j++)
            nodesIdx[j] = j;
        firstRun = false;
    }

    int newIdx = listsIdx[listsIdxHead];
    lists[newIdx].size = 0;
    lists[newIdx].listIdx = newIdx;
    lists[newIdx].currentNodeCounter = 0;
    lists[newIdx].headNode = NULL;
    lists[newIdx].currentNode = NULL;
    lists[newIdx].tailNode = NULL;
    listsIdxHead++;

    return &(lists[listsIdxHead - 1]);
}

int List_count(List* pList) {
    return pList->size;
}

void* List_first(List* pList) {
    if (pList->size == 0) return NULL;

    pList->currentNode = pList->headNode;
    pList->currentNodeCounter = 1;

    return (pList->currentNode)->data;
}

void* List_last(List* pList) {
    if (pList->size == 0) return NULL;

    pList->currentNode = pList->tailNode;
    pList->currentNodeCounter = pList->size;

    return (pList->currentNode)->data;
}

void* List_next(List* pList) {
    if (pList->size == 0) return NULL;

    if (pList->currentNode == NULL) {
        if (pList->currentNodeCounter == 0) {
            pList->currentNode = pList->headNode;
            pList->currentNodeCounter = 1;
            return (pList->currentNode)->data;
        } else {
            return NULL;
        }
    }
    pList->currentNode = pList->currentNode->next;
    pList->currentNodeCounter += 1;

    if (pList->currentNode == NULL) return NULL;
    return (pList->currentNode)->data;
}

void* List_prev(List* pList) {
    if (pList->size == 0) return NULL;

    if (pList->currentNode == NULL) {
        if (pList->currentNodeCounter != 0) {
            pList->currentNode = pList->tailNode;
            pList->currentNodeCounter = pList->size;
            return (pList->currentNode)->data;
        } else {
            return NULL;
        }
    }
    pList->currentNode = pList->currentNode->prev;
    pList->currentNodeCounter -= 1;

    if (pList->currentNode == NULL) return NULL;
    return (pList->currentNode)->data;
}

void* List_curr(List* pList) {
    if (pList->currentNode == NULL) return NULL;
    return (pList->currentNode)->data;
}

int List_insert_after(List* pList, void* pItem) {
    if (nodesIdxHead >= LIST_MAX_NUM_NODES) return LIST_FAIL;

    int newNodeIdx = nodesIdx[nodesIdxHead];
    nodesIdxHead++;
    nodes[newNodeIdx].nodeIdx = newNodeIdx;
    nodes[newNodeIdx].data = pItem;

    if (pList->currentNodeCounter == 0) {
        nodes[newNodeIdx].prev = NULL;
        nodes[newNodeIdx].next = pList->headNode;
        pList->currentNodeCounter = 1;
        if (pList->size == 0) { 
            pList->tailNode = &(nodes[newNodeIdx]);
        } else {
            (pList->headNode)->prev = &(nodes[newNodeIdx]);
        }
        pList->headNode = &(nodes[newNodeIdx]);
    } else if (pList->currentNodeCounter > pList->size) {
        nodes[newNodeIdx].next = NULL;
        nodes[newNodeIdx].prev = pList->tailNode;
        pList->currentNodeCounter = (pList->size) + 1;
        (pList->tailNode)->next = &(nodes[newNodeIdx]);
        pList->tailNode = &(nodes[newNodeIdx]);
    } else {
        nodes[newNodeIdx].prev = pList->currentNode;
        nodes[newNodeIdx].next = pList->currentNode->next;
        if (pList->currentNode->next != NULL) {
            (pList->currentNode->next)->prev = &(nodes[newNodeIdx]);
        }
        if (pList->currentNodeCounter == pList->size) {
            pList->tailNode = &(nodes[newNodeIdx]);
        }
        (pList->currentNode)->next = &(nodes[newNodeIdx]);
        pList->currentNodeCounter += 1;
    }
    pList->currentNode = &(nodes[newNodeIdx]);
    pList->size += 1;

    return LIST_SUCCESS;
}

int List_insert_before(List* pList, void* pItem) {
    if (nodesIdxHead >= LIST_MAX_NUM_NODES) return LIST_FAIL;

    int newNodeIdx = nodesIdx[nodesIdxHead];
    nodesIdxHead++;
    nodes[newNodeIdx].nodeIdx = newNodeIdx;
    nodes[newNodeIdx].data = pItem;

    if (pList->currentNodeCounter == 0) {
        nodes[newNodeIdx].prev = NULL;
        nodes[newNodeIdx].next = pList->headNode;
        pList->currentNodeCounter = 1;
        if (pList->size == 0) { 
            pList->tailNode = &(nodes[newNodeIdx]);
        } else {
            (pList->headNode)->prev = &(nodes[newNodeIdx]);
        }
        pList->headNode = &(nodes[newNodeIdx]);
    } else if (pList->currentNodeCounter > pList->size) {
        nodes[newNodeIdx].next = NULL;
        nodes[newNodeIdx].prev = pList->tailNode;
        pList->currentNodeCounter = (pList->size) + 1;
        pList->tailNode->next = &(nodes[newNodeIdx]);
        pList->tailNode = &(nodes[newNodeIdx]);
    } else {
        nodes[newNodeIdx].next = pList->currentNode;
        nodes[newNodeIdx].prev = pList->currentNode->prev;
        if ((pList->currentNode->prev) != NULL) {
            (pList->currentNode->prev)->next = &(nodes[newNodeIdx]);
        }
        if (pList->currentNodeCounter == 1) {
            pList->headNode = &(nodes[newNodeIdx]);
        }
        (pList->currentNode)->prev = &(nodes[newNodeIdx]);
    }
    pList->currentNode = &(nodes[newNodeIdx]);
    pList->size += 1;

    return LIST_SUCCESS;
}

int List_append(List* pList, void* pItem) { 
    if (nodesIdxHead >= LIST_MAX_NUM_NODES) return LIST_FAIL;

    int newNodeIdx = nodesIdx[nodesIdxHead];
    nodesIdxHead++;
    nodes[newNodeIdx].nodeIdx = newNodeIdx;
    nodes[newNodeIdx].data = pItem;
    nodes[newNodeIdx].next = NULL;
    nodes[newNodeIdx].prev = pList->tailNode;

    pList->currentNode = &(nodes[newNodeIdx]);
    if (pList->tailNode != NULL) {
        pList->tailNode->next = pList->currentNode;
    } else {
        pList->headNode = pList->currentNode;
    }
    pList->tailNode = pList->currentNode;
    pList->size += 1;
    pList->currentNodeCounter = pList->size;

    return LIST_SUCCESS;
}

int List_prepend(List* pList, void* pItem) {
    if (nodesIdxHead >= LIST_MAX_NUM_NODES) return LIST_FAIL;

    int newNodeIdx = nodesIdx[nodesIdxHead];
    nodesIdxHead++;
    nodes[newNodeIdx].nodeIdx = newNodeIdx;
    nodes[newNodeIdx].data = pItem;
    nodes[newNodeIdx].next = pList->headNode;
    nodes[newNodeIdx].prev = NULL;

    pList->currentNode = &(nodes[newNodeIdx]);
    if (pList->headNode != NULL) {
        pList->headNode->prev = &(nodes[newNodeIdx]);
    } else {
        pList->tailNode = pList->currentNode;
    }
    pList->headNode = pList->currentNode;
    pList->size += 1;
    pList->currentNodeCounter = 1;

    return LIST_SUCCESS;
}

void* List_remove(List* pList) {
    if (pList->currentNode == NULL) return NULL;
    
    Node* itemToRemove = pList->currentNode; 
    if (itemToRemove->prev != NULL) {
        (itemToRemove->prev)->next = itemToRemove->next;
    }
    if (itemToRemove->next != NULL) {
        pList->currentNode = itemToRemove->next;
        (pList->currentNode)->prev = itemToRemove->prev;
        if (pList->currentNodeCounter == 1) {
            pList->headNode = pList->currentNode;
        }
    }
    if (itemToRemove == pList->tailNode) {
        if (pList->size == 1) {
            pList->currentNodeCounter = 0;
            pList->headNode = NULL;
            pList->tailNode = NULL;
        } else {
            pList->tailNode = pList->currentNode->prev;
            pList->currentNodeCounter = pList->size;
        }
        pList->currentNode = NULL;
    }

    nodesIdxHead--;
    nodesIdx[nodesIdxHead] = itemToRemove->nodeIdx;
    pList->size -= 1;

    return itemToRemove->data;
}

void* List_trim(List* pList) {
    if (pList->size == 0) return NULL;

    Node* itemToRemove = pList->tailNode;
    if (pList->size == 1) {
        pList->currentNode = NULL;
        pList->headNode = NULL;
        pList->tailNode = NULL;
    } else {
        pList->currentNode = itemToRemove->prev;
        pList->currentNode->next = NULL;
        pList->tailNode = pList->currentNode;
        if (pList->size == 2) {
            pList->headNode == pList->currentNode;
        }
    }

    nodesIdxHead--;
    nodesIdx[nodesIdxHead] = itemToRemove->nodeIdx;
    pList->size -= 1;
    pList->currentNodeCounter = pList->size;

    return itemToRemove->data;
}

void List_concat(List* pList1, List* pList2) {
    if (pList2->size == 0) {
        listsIdxHead--;
        listsIdx[listsIdxHead] = pList2->listIdx;
        return;
    }

    if (pList1->size == 0) {
        pList1->size = pList2->size;
        pList1->headNode = pList2->headNode;
    } else {
        (pList1->tailNode)->next = pList2->headNode;
        (pList2->headNode)->prev = pList1->tailNode;
        pList1->size = pList1->size + pList2->size;
    }

    pList1->tailNode = pList2->tailNode;
    listsIdxHead--;
    listsIdx[listsIdxHead] = pList2->listIdx;
}

void List_free(List* pList, FREE_FN pItemFreeFn) {
    pList->currentNode = pList->headNode;

    while (pList->currentNode != NULL) {
        (*pItemFreeFn)(pList->currentNode->data);
        nodesIdxHead--;
        nodesIdx[nodesIdxHead] = pList->currentNode->nodeIdx;
        pList->currentNode = pList->currentNode->next;
    }

    listsIdxHead--;
    listsIdx[listsIdxHead] = pList->listIdx;
}

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    if (pList->currentNodeCounter == 0 && pList->size != 0) List_next(pList);

    while (pList->currentNode != NULL) {
        if ((*pComparator)(pList->currentNode->data, pComparisonArg) == 1) {
            return pList->currentNode->data;
        }
        pList->currentNode = pList->currentNode->next;
        pList->currentNodeCounter += 1;
    }
    
    return NULL;
}