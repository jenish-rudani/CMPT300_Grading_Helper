#include <stdio.h>
#include <assert.h>
#include "list.h"

static List head_pool[LIST_MAX_NUM_HEADS];
static Node node_pool[LIST_MAX_NUM_NODES];

// to manage the free List structure
// and Node structure. We will create two stacks
// of free heads and Nodes. It will help us to get the free
// nodes in O(1) (without any searching through the entire array)

typedef struct Stack_s Stack;
struct Stack_s
{
    void *data[LIST_MAX_NUM_NODES];
    int pointer;
};

void push(Stack *stack, void *data)
{
    if (stack->pointer >= LIST_MAX_NUM_NODES)
        return;
    stack->data[stack->pointer++] = data;
}

void *pop(Stack *stack)
{
    if (stack->pointer == 0)
        return NULL;

    (stack->pointer)--;
    void *data = stack->data[stack->pointer];
    return data;
}

static Stack head_stack, node_stack;
static bool structures_initialized = false;

// initializes all the variables
// of a List structure
static void init_list(List *l)
{
    l->head = NULL;
    l->tail = NULL;
    l->cur = NULL;
    l->size = 0;
    l->bound = LIST_OOB_END;
}

// initializes all the variables
// of a Node structure
static void init_node(Node *l)
{
    l->next = NULL;
    l->prev = NULL;
    l->data = NULL;
}

// creates two list of heads and nodes
// two manage the free nodes efficiently
static void init_structures()
{
    structures_initialized = true;
    head_stack.pointer = 0;
    node_stack.pointer = 0;

    for (int i = 0; i < LIST_MAX_NUM_HEADS; i++)
    {
        init_list(&head_pool[i]);
        push(&head_stack, &head_pool[i]);
    }

    for (int i = 0; i < LIST_MAX_NUM_NODES; i++)
    {
        init_node(&node_pool[i]);
        push(&node_stack, &node_pool[i]);
    }
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List *List_create()
{
    if (!structures_initialized)
        init_structures();

    List *list = pop(&head_stack);

    if (list == NULL)
        return NULL;

    init_list(list);

    return list;
}

// Returns the number of items in pList.
int List_count(List *pList)
{
    assert(pList != NULL);
    return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void *List_first(List *pList)
{
    assert(pList != NULL);

    pList->cur = pList->head;
    if (pList->cur == NULL)
        return NULL;

    pList->bound = -1;
    return pList->cur->data;
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void *List_last(List *pList)
{
    assert(pList != NULL);

    pList->cur = pList->tail;

    if (pList->cur == NULL)
        return NULL;

    pList->bound = -1;
    return pList->cur->data;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer
// is returned and the current item is set to be beyond end of pList.
void *List_next(List *pList)
{
    if (pList->bound != -1)
        return NULL;

    pList->cur = pList->cur->next;
    if (pList->cur == NULL)
    {
        pList->bound = LIST_OOB_END;
        return NULL;
    }
    return pList->cur->data;
}

// Backs up pList's current item by one, and returns a pointer to the new current item.
// If this operation backs up the current item beyond the start of the pList, a NULL pointer
// is returned and the current item is set to be before the start of pList.
void *List_prev(List *pList)
{
    if (pList->bound != -1)
        return NULL;

    pList->cur = pList->cur->prev;
    if (pList->cur == NULL)
    {
        pList->bound = LIST_OOB_START;
        return NULL;
    }
    return pList->cur->data;
}

// Returns a pointer to the current item in pList.
void *List_curr(List *pList)
{
    if (pList->bound != -1)
        return NULL;

    return pList->cur->data;
}

// Adds the new item to pList directly after the current item, and makes item the current item.
// If the current pointer is before the start of the pList, the item is added at the start. If
// the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert_after(List *pList, void *pItem)
{
    if (pList->bound == LIST_OOB_START)
        return List_prepend(pList, pItem);

    if (pList->bound == LIST_OOB_END)
        return List_append(pList, pItem);

    Node *node = pop(&node_stack);
    if (node == NULL)
        return -1;

    node->data = pItem;
    node->prev = NULL;
    node->next = NULL;

    if (pList->cur == pList->tail)
    {
        node->prev = pList->tail;
        pList->tail->next = node;
        pList->tail = node;
    }
    else
    {
        node->prev = pList->cur;
        node->next = pList->cur->next;

        pList->cur->next->prev = node;
        pList->cur->next = node;
    }

    pList->cur = pList->cur->next;
    pList->size++;
    return 0;
}

// Adds item to pList directly before the current item, and makes the new item the current one.
// If the current pointer is before the start of the pList, the item is added at the start.
// If the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert_before(List *pList, void *pItem)
{
    if (pList->bound == LIST_OOB_START)
        return List_prepend(pList, pItem);

    if (pList->bound == LIST_OOB_END)
        return List_append(pList, pItem);

    Node *node = pop(&node_stack);
    if (node == NULL)
        return -1;

    node->data = pItem;
    node->prev = NULL;
    node->next = NULL;

    if (pList->cur == pList->head)
    {
        node->next = pList->head;
        pList->head->prev = node;

        pList->head = node;
    }
    else
    {
        node->prev = pList->cur->prev;
        node->next = pList->cur;

        pList->cur->prev->next = node;
        pList->cur->prev = node;
    }

    pList->cur = pList->cur->prev;
    pList->size++;
    return 0;
}

// Adds item to the end of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_append(List *pList, void *pItem)
{

    Node *node = pop(&node_stack);
    if (node == NULL)
        return -1;

    node->data = pItem;
    node->next = NULL;
    node->prev = NULL;

    pList->size++;
    pList->bound = -1;

    if (pList->tail == NULL)
    {
        pList->head = pList->tail = pList->cur = node;
        return 0;
    }

    pList->tail->next = node;
    node->prev = pList->tail;

    pList->tail = pList->cur = node;
    return 0;
}

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_prepend(List *pList, void *pItem)
{
    Node *node = pop(&node_stack);
    if (node == NULL)
        return -1;

    node->data = pItem;
    node->next = NULL;
    node->prev = NULL;

    pList->size++;
    pList->bound = -1;

    if (pList->tail == NULL)
    {
        pList->head = pList->tail = pList->cur = node;
        return 0;
    }

    pList->head->prev = node;
    node->next = pList->head;

    pList->head = pList->cur = node;
    return 0;
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void *List_remove(List *pList)
{

    if (pList->bound != -1)
        return NULL;

    pList->size--;

    void *data = pList->cur->data;
    push(&node_stack, pList->cur);

    if (pList->head == pList->tail)
    {
        pList->head = pList->tail = pList->cur = NULL;
        pList->bound = LIST_OOB_END;

        return data;
    }

    if (pList->cur == pList->tail)
    {
        pList->tail = pList->tail->prev;
        pList->tail->next = NULL;

        pList->cur = NULL;
        pList->bound = LIST_OOB_END;
        return data;
    }

    if (pList->cur == pList->head)
    {
        pList->head = pList->head->next;
        pList->head->prev = NULL;

        pList->cur = pList->head;
        return data;
    }

    Node *prev = pList->cur->prev;
    Node *next = pList->cur->next;

    prev->next = next;
    next->prev = prev;

    pList->cur = next;

    return data;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void *List_trim(List *pList)
{

    if (pList->head == NULL)
        return NULL;

    pList->size--;
    void *data = pList->tail->data;
    push(&node_stack, pList->tail);

    if (pList->head == pList->tail)
    {
        pList->head = pList->tail = pList->cur = NULL;
        pList->bound = LIST_OOB_END;
    }
    else
    {

        pList->tail = pList->tail->prev;
        pList->tail->next = NULL;
        pList->cur = pList->tail;
        pList->bound = -1;
    }
    return data;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1.
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List *pList1, List *pList2)
{
    assert(pList1 != NULL && pList2 != NULL);

    if (pList2 != NULL)
        push(&head_stack, pList2);

    if (pList1->head == NULL && pList2->head == NULL)
    {

        return;
    }
    else if (pList1->head == NULL)
    {
        pList1->head = pList2->head;
        pList1->tail = pList2->tail;
        pList1->size = pList2->size;
        pList1->cur = NULL;
        pList1->bound = LIST_OOB_START;
        return;
    }
    else if (pList2->head == NULL)
        return;

    pList1->tail->next = pList2->head;
    pList2->head->prev = pList1->tail;
    pList1->tail = pList2->tail;
    pList1->size += pList2->size;

    return;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are
// available for future operations.
void List_free(List *pList, FREE_FN pItemFreeFn)
{

    Node *cur = pList->head;

    while (cur != NULL)
    {
        push(&node_stack, cur);
        (*pItemFreeFn)(cur->data);
        cur = cur->next;
    }
    push(&head_stack, pList);
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

void *List_search(List *pList, COMPARATOR_FN pComparator, void *pComparisonArg)
{

    if (pList->bound != -1)
    {
        pList->bound = LIST_OOB_END;
        return NULL;
    }

    Node *curr = pList->cur;

    while (curr != NULL)
    {
        // printf("sdf: %d\n", *((int*)curr->data));
        fflush(stdout);
        if ((*pComparator)(curr->data, pComparisonArg) == 1)
        {
            pList->cur = curr;
            return curr->data;
        }

        curr = curr->next;
    }

    pList->bound = LIST_OOB_END;
    pList->cur = NULL;
    return NULL;
}