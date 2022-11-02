/*
Doubly linked list implementation.
All datatypes are statically allocated
and can be found in constant time. 
*/
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static Node nodes[LIST_MAX_NUM_NODES];
static List lists[LIST_MAX_NUM_HEADS];

// Pointers to free node and free list_head (with reset values).
// Used to allocate nodes and list_heads in constant time.
// Values set in init_nodes() and init_list_heads().
static Node *pFreeNode;
static List *pFreeList;

static bool initialized = false;

// Default values of node.
void reset_node(Node *pNode) {
    pNode->next = NULL;
    pNode->prev = NULL;
    pNode->item = NULL;
}

// Default values of list_head.
void reset_list_head(List *list) {
    list->first = NULL;
    list->last = NULL;
    list->current_node = NULL;
}

// When a node or list_head struct is allocated, 
// the pointer to the free struct (pFreeNode or pFreeList) 
// becomes the value of the struct's _next_free field.
// These initialization functions set _next_free to make a chain of structs.
void init_nodes() {
    for (int i = 0; i < LIST_MAX_NUM_NODES - 1; i++) {
        reset_node(&nodes[i]);
        nodes[i]._next_free = &nodes[i + 1];
    }
    reset_node(&nodes[LIST_MAX_NUM_NODES - 1]);
    nodes[LIST_MAX_NUM_NODES - 1]._next_free = NULL;
    pFreeNode = nodes;
}

void init_list_heads() {
    for (int i = 0; i < LIST_MAX_NUM_HEADS - 1; i++) {
        reset_list_head(&lists[i]);
        lists[i]._next_free = &lists[i + 1];
    }
    reset_list_head(&lists[LIST_MAX_NUM_HEADS - 1]);
    lists[LIST_MAX_NUM_HEADS - 1]._next_free = NULL;
    pFreeList = lists;
}

void init() {
    initialized = true;
    init_nodes();
    init_list_heads();
}

void free_node(Node *pNode) {
    if (pFreeNode) {
        pNode->_next_free = pFreeNode;
    } else {
        pNode->_next_free = NULL;
    }
    reset_node(pNode);
    pFreeNode = pNode;
}

void free_list_head(List *pList) {
    if (pFreeList) {
        pList->_next_free = pFreeList;
    } else {
        pList->_next_free = NULL;
    }
    reset_list_head(pList);
    pFreeList = pList;
}

void attach(Node *n, Node *nplus) {
    n->next = nplus;
    nplus->prev = n;
}

Node *Node_create(void *item) {
    if (!initialized) {
        init();
    }
    if (pFreeNode) {
        Node *pAllocated = pFreeNode;
        pFreeNode = pFreeNode->_next_free;
        pAllocated->item = item;
        return pAllocated;
    }
    return NULL;
}

enum ListOutOfBounds List_OOB_state(List *pList) {
    assert(pList);
    if (pList->current_node == NULL && List_count(pList) != 0) {
        if (pList->OOB_END_flag) {
            return LIST_OOB_END;
        } else {
            return LIST_OOB_START;
        }
    }
    return LIST_OOB_NA;
}

//----- implementations of functions in list.h -----//

List *List_create() {
    if (!initialized) {
        init();
    }
    if (pFreeList) {
        List *pAllocated = pFreeList;
        pFreeList = pFreeList->_next_free;
        return pAllocated;
    }
    return NULL;
}

int List_count(List *pList) {
    assert(pList);
    int count = 0;
    Node *current = pList->first;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

void *List_first(List *pList) {
    assert(pList);
    pList->current_node = pList->first;
    if (pList->first == NULL) {
        return NULL;
    }
    return pList->first->item;
}

void *List_last(List *pList) {
    assert(pList);
    pList->current_node = pList->last;
    if (pList->last == NULL) {
        return NULL;
    }
    return pList->last->item;
}

void *List_next(List *pList) {
    assert(pList);    
    enum ListOutOfBounds OOB_state = List_OOB_state(pList);
    bool at_last = pList->current_node == pList->last;
    if (at_last) {
        pList->current_node = NULL;
        pList->OOB_END_flag = true;
    }
    if (List_count(pList) == 0 || OOB_state == LIST_OOB_END || at_last) {
        return NULL;
    }

    if (OOB_state == LIST_OOB_START) {
        pList->current_node = pList->first;
    } else {
        pList->current_node = pList->current_node->next;
    }
    return pList->current_node->item;
}

void *List_prev(List *pList) {
    assert(pList);
    enum ListOutOfBounds OOB_state = List_OOB_state(pList);
    bool at_first = pList->current_node == pList->first;
    if (at_first) {
        pList->current_node = NULL;
        pList->OOB_END_flag = false;
    }
    if (List_count(pList) == 0 || OOB_state == LIST_OOB_START || at_first) {
        return NULL;
    }

    if (OOB_state == LIST_OOB_END) {
        pList->current_node = pList->last;
    } else {
        pList->current_node = pList->current_node->prev;
    }
    return pList->current_node->item;
}

void *List_curr(List *pList) {
    if (pList->current_node) {
        return pList->current_node->item;
    }
    return NULL;
}

int List_insert_oob(List *pList, void *pItem) {
    // current_node beyond end of pList, item added to the end.
    if (pList->OOB_END_flag) {
        return List_append(pList, pItem);
    }
    // current_node before start of pList, item added to the start.
    return List_prepend(pList, pItem);
}

int List_insert_after(List *pList, void *pItem) {
    if (List_OOB_state(pList) != LIST_OOB_NA) {
        return List_insert_oob(pList, pItem);
    }
    // Catches lists of count 0 and 1, 
    // and when current node is last node.
    if (pList->current_node == pList->last) {
        return List_append(pList, pItem);
    }

    // 'Inserting after' between (inclusive) 'first' and 'second last' element.
    assert(pList);
    Node *pNodeNew = Node_create(pItem);
    if (!pNodeNew) {
        return LIST_FAIL; // No free nodes.
    }
    Node *current_next = pList->current_node->next;
    attach(pList->current_node, pNodeNew);
    attach(pNodeNew, current_next);
    pList->current_node = pNodeNew;
    return LIST_SUCCESS;
}

int List_insert_before(List *pList, void *pItem) {
    if (List_OOB_state(pList) != LIST_OOB_NA) {
        return List_insert_oob(pList, pItem);
    }
    // Catches lists of count 0 and 1, 
    // and when current node is last node.
    if (pList->current_node == pList->first) {
        return List_prepend(pList, pItem);
    }

    // 'Inserting before' between (inclusive) 'second' and 'last' element.
    assert(pList);
    Node *pNodeNew = Node_create(pItem);
    if (!pNodeNew) {
        return LIST_FAIL; // No free nodes.
    }
    Node *current_prev = pList->current_node->prev;
    attach(current_prev, pNodeNew);
    attach(pNodeNew, pList->current_node);
    pList->current_node = pNodeNew;
    return LIST_SUCCESS;
}

int List_append(List *pList, void *pItem) {
    assert(pList);
    Node *pNodeNew = Node_create(pItem);
    if (!pNodeNew) {
        return LIST_FAIL; // No free nodes.
    }
    int count = List_count(pList);
    if (count == 0) {
        pList->first = pNodeNew;
    } else {
        attach(pList->last, pNodeNew);
    }
    pList->last = pNodeNew;
    pList->current_node = pNodeNew;
    return LIST_SUCCESS;
}

int List_prepend(List *pList, void *pItem) {
    assert(pList);
    Node *pNodeNew = Node_create(pItem);
    if (!pNodeNew) {
        return LIST_FAIL; // No free nodes.
    }
    int count = List_count(pList);
    if (count == 0) {
        pList->last = pNodeNew;
    } else {
        attach(pNodeNew, pList->first);
    }
    pList->first = pNodeNew;
    pList->current_node = pNodeNew;
    return LIST_SUCCESS;
}

void *List_remove(List *pList) {
    assert(pList);
    int count = List_count(pList);

    // out of bounds or size 0
    if (List_OOB_state(pList) != LIST_OOB_NA || count == 0) {
        return NULL;
    }

    Node *node_to_free = pList->current_node;
    void *item = node_to_free->item;

    if (node_to_free == pList->first) {
        pList->first = pList->first->next;
        if (pList->first) { // Check necessary if lists of count 1.
            pList->first->prev = NULL;
        }
    } 
    else if (node_to_free == pList->last) {
        pList->last = pList->last->prev;
        pList->last->next = NULL;
        pList->OOB_END_flag = true; // When pList->current is shifted right, will be OOB.
    }
    // Note: count > 2 here, since node isn't head or tail.
    else {
        attach(node_to_free->prev, node_to_free->next);
    }

    pList->current_node = node_to_free->next;
    free_node(node_to_free);
    return item;
}

void *List_trim(List *pList) {
    assert(pList);
    if (List_count(pList) == 0) {
        return NULL;
    }

    Node *node_to_free = pList->last;
    void *item = node_to_free->item;

    if (List_count(pList) == 1) {
        pList->first = NULL;
        pList->last = NULL;
    } else {
        pList->last = pList->last->prev;
        pList->last->next = NULL;
    }
    
    free_node(node_to_free);
    return item;
}

void List_concat(List *pList1, List *pList2) {
    assert(pList1);
    assert(pList2);
    if (List_count(pList1) == 0) {
        pList1->first = pList2->first;
    } else {
        pList1->last->next = pList2->first;
    }
    pList1->last = pList2->last;
    free_list_head(pList2);    
}

void List_free(List *pList, FREE_FN pItemFreeFn) {
    assert(pList);
    Node *current = pList->first;
    while (current) {
        Node *next_node = current->next;
        pItemFreeFn(current->item);
        free_node(current);
        current = next_node;
    }
    free_list_head(pList);
}

void *List_search(List *pList, COMPARATOR_FN pComparator, void *pComparisonArg) {
    // Search from current item. 
    // Return pointer to matching item and sets current pointer to be
    // at matching node.
    assert(pList);
    bool before_start = List_OOB_state(pList) == LIST_OOB_START;
    Node *node = before_start ? pList->first : pList->current_node;
    while (node) {
        if (pComparator(node->item, pComparisonArg)) { 
            pList->current_node = node;
            return node->item;
        }
        node = node->next;
    }
    // No match found.
    pList->current_node = NULL;
    pList->OOB_END_flag = true;
    return NULL;
}
