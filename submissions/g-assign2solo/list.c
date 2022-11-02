#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "list.h"

// Need to seperate linked lists into used and unused sections
// Used sections will be referred to when creating
// Unused sections will be referred to when deleting

static List lists[LIST_MAX_NUM_HEADS];
static Node nodes[LIST_MAX_NUM_NODES];
static List* unusedListHead;
static List* unusedListTail;
static Node* unusedNodeHead;
static Node* unusedNodeTail;
static int usedListCount = 0;
static int usedNodeCount = 0;
static bool firstRun = true;

static Node* Node_create();
static void Initialize();
static void List_delete();

// Creates a new list after latest one in lists[]
List* List_create(){
    if (firstRun){
        Initialize();
    }

    if (usedListCount < LIST_MAX_NUM_HEADS) {
        List *newList;
        Node *newNode = Node_create(); // The head node for the list

        // If it's the first list
        if(firstRun){
            // Set current list to position 0 on array
            newList = &lists[0];

            newList->head = newNode;
            newList->tail = newNode;
            newList->current = newNode;
            newList->currentItem = newNode->item;
            newList->itemCount = 0;
            
            unusedListHead = newList->next;
            firstRun = false;
        } else {
            // Set current list to the next available unused list
            List* previousList = unusedListHead->prev;
            newList = unusedListHead;

            newList->head = newNode;
            newList->tail = newNode;
            newList->current = newNode;
            newList->currentItem = newNode->item;
            newList->prev = unusedListHead->prev;
            previousList->next = newList;
            newList->itemCount = 0;
            
            if(newList->next != NULL){
                unusedListHead = newList->next;
            } else {
                unusedListHead = newList;
                unusedListTail = newList;
            }
        }
        
        printf("List %d created \n", usedListCount);
        
        usedListCount++;
        return newList;
    } else {
        printf("\nList Capacity Reached\n\n");
    }

    return NULL;
}

// Creates a new node after latest one in nodes[]
static Node* Node_create(){
    if (usedNodeCount < LIST_MAX_NUM_NODES) {
        Node* newNode;

        // If the node is the head
        if (firstRun){
            // Sets current node to position 0 on array
            newNode = &nodes[0];
            unusedNodeHead = newNode->next;
        } else {
            // Sets current node to next one after current used tail
            newNode = unusedNodeHead;
            unusedNodeHead = newNode->next;
        }

        usedNodeCount++;
        return newNode;
    }

    return NULL;
}

// Fills arrays with nodes when app is first run
static void Initialize(){
    int i = 0, j = 0;

    // Populates list array
    while (i < LIST_MAX_NUM_HEADS) {
        List list;

        // Adds next and prev reference unless not applicable
        if(i > 0 && i < LIST_MAX_NUM_HEADS - 1){ // Not tail or head
            list.head = NULL;
            list.tail = NULL;
            list.current = NULL;
            list.currentItem = NULL;
            lists[i-1].next = &lists[i];
            list.prev = &lists[i-1];
            list.oobStatus = -1;
            list.itemCount = 0;
        } else if (i == 0) { // Is head
            list.head = NULL;
            list.tail = NULL;
            list.current = NULL;
            list.currentItem = NULL;
            list.next = NULL;
            list.prev = NULL;
            list.oobStatus = -1;
            list.itemCount = 0;
            unusedListHead = &lists[0];
        } else if (i == LIST_MAX_NUM_HEADS - 1) { // Is tail
            list.head = NULL;
            list.tail = NULL;
            list.current = NULL;
            list.currentItem = NULL;
            lists[i-1].next = &lists[i];
            list.prev = &lists[i-1];
            list.oobStatus = -1;
            list.itemCount = 0;
            unusedListTail = &lists[LIST_MAX_NUM_HEADS - 1];
            list.next = NULL;
            //lists[0].prev = unusedListTail;
        } else {
            printf("List Iterator %d out of bounds \n", i);
            assert(i < LIST_MAX_NUM_HEADS);
            assert(i > 0);
        }

        lists[i] = list;
        i++;
    }

    // Populates node array
    while (j < LIST_MAX_NUM_NODES) {
        Node node;
        
        // Adds next reference unless it's the tail
        if (j > 0 && j < LIST_MAX_NUM_NODES - 1) { // Not tail or head
            node.item = NULL;
            nodes[j-1].next = &nodes[j];
            node.prev = &nodes[j-1];
        } else if (j == 0) { // Head
            node.item = NULL;
            node.next = NULL;
            node.prev = NULL;
            unusedNodeHead = &nodes[0];
        } else if (j == LIST_MAX_NUM_NODES - 1) { // Tail
            node.item = NULL;
            nodes[j-1].next = &nodes[j];
            node.prev = &nodes[j-1];
            unusedNodeTail = &nodes[LIST_MAX_NUM_NODES - 1];
            node.next = unusedNodeHead;
            nodes[0].prev = unusedNodeTail;
        } else {
            printf("Node Iterator %d out of bounds \n", j);
            assert(i < LIST_MAX_NUM_NODES);
            assert(i > 0);
        }

        nodes[j] = node;
        j++;
    }
}

// Returns number of items in pList
int List_count(List* pList){
    return pList->itemCount;
}

// Returns head
void* List_first(List* pList){
    if (pList->head->item == NULL || pList->head == NULL){
        pList->currentItem = NULL;
        return NULL;
    }

    return pList->head->item;
}

// Returns tail
void* List_last(List* pList){
    if (pList->tail->item == NULL || pList->tail == NULL){
        pList->currentItem = NULL;
        return NULL;
    }

    return pList->tail->item;
}

// Moves current node pointer forward in list
void* List_next(List* pList){
    if (pList->itemCount == 0)
        return NULL;

    if (pList->current->prev == pList->tail || pList->oobStatus == LIST_OOB_END){
        pList->currentItem = NULL;
        pList->oobStatus = LIST_OOB_END;
        return NULL;
    } else {
        if(pList->current == pList->tail){
            pList->currentItem = NULL;
            pList->oobStatus = LIST_OOB_END;
            return NULL;
        }

        if (pList->oobStatus == LIST_OOB_START)
            pList->oobStatus = -1;

        pList->current = pList->current->next;
        pList->currentItem = pList->current->item;

        return pList->currentItem;
    }
}

// Moves current node pointer backward in list
void* List_prev(List* pList){
    if (pList->itemCount == 0)
        return NULL;

    if (pList->oobStatus == LIST_OOB_START){
        pList->currentItem = NULL;
        pList->oobStatus = LIST_OOB_START;
        return NULL;
    } else {
        if (pList->oobStatus == LIST_OOB_END)
            pList->oobStatus = -1;

        pList->current = pList->current->prev;

        // Check if OOB behind head
        if(pList->current != NULL && pList->current != pList->head->prev){
            pList->currentItem = pList->current->item;
        } else {
            pList->currentItem = NULL;
            pList->oobStatus = LIST_OOB_START;

            return NULL;
        }

        return pList->currentItem;
    }
}

// Returns current item
void* List_curr(List* pList){
    return pList->currentItem;
}

// Add new item after current selected node
int List_insert_after(List* pList, void* pItem){
    if (usedNodeCount < LIST_MAX_NUM_NODES) {
        if(pList->current == pList->head && pList->current == pList->tail && pList->current->item == NULL){
            pList->current->item = pItem;
            pList->currentItem = pItem;
            pList->itemCount++;

            return LIST_SUCCESS;
        } else if (pList->oobStatus == LIST_OOB_START) {
            Node *newNode = Node_create();
            pList->itemCount++;
            pList->oobStatus = -1;
            newNode->item = pItem;

            if (pList->head != newNode->next){
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;
                pList->head->prev = newNode;
            }


           if (pList->prev != NULL && pList->prev->tail != NULL) {
                newNode->prev = pList->prev->tail;
                pList->prev->tail->next = newNode;
            } else {
                if (newNode != unusedNodeTail) {
                    newNode->prev = unusedNodeTail;
                    unusedNodeTail->next = newNode;
                }
            }

            newNode->next = pList->head;

            pList->head = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            return LIST_SUCCESS;
        } else if (pList->oobStatus == LIST_OOB_END) {
            Node *newNode = Node_create();
            pList->itemCount++;
            pList->oobStatus = -1;
            newNode->item = pItem;

            if (pList->tail != newNode->prev){
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;
                pList->tail->next = newNode;
            }

            if(pList->next != NULL && pList->next->head != NULL){
                newNode->next = pList->next->head;
                pList->next->head->prev = newNode;
            } else {
                if (newNode != unusedNodeHead) {
                    newNode->next = unusedNodeHead;
                    unusedNodeHead->prev = newNode;
                }
            }

            newNode->prev = pList->tail;

            pList->tail = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            return LIST_SUCCESS;
        } else {
            Node *newNode = Node_create();
            pList->itemCount++;
            newNode->item = pItem;

            if (pList->current->next != newNode) {
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;

                newNode->next = pList->current->next;
                newNode->prev = pList->current;
            }

            if (pList->current == pList->tail) {
                if(pList->next != NULL && pList->next->head != NULL){
                    newNode->next = pList->next->head;
                    pList->next->head->prev = newNode;
                } else {
                    if (newNode != unusedNodeHead) {
                        newNode->next = unusedNodeHead;
                        unusedNodeHead->prev = newNode;
                    }
                }
                pList->tail = newNode;
            }

            pList->current->next->prev = newNode;
            pList->current->next = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            return LIST_SUCCESS;
        }
    }

    return LIST_FAIL;
}

// Add new item before current selected node
int List_insert_before(List* pList, void* pItem){
    if (usedNodeCount < LIST_MAX_NUM_NODES) {
        if(pList->current == pList->head && pList->current == pList->tail && pList->current->item == NULL){
            pList->current->item = pItem;
            pList->currentItem = pItem;
            pList->itemCount++;
            
            return LIST_SUCCESS;
        } else if (pList->oobStatus == LIST_OOB_START) {
            Node *newNode = Node_create();
            pList->itemCount++;
            pList->oobStatus = -1;
            newNode->item = pItem;

            if (pList->head != newNode->next){
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;
                pList->head->prev = newNode;
            }

            if (pList->prev != NULL && pList->prev->tail != NULL) {
                newNode->prev = pList->prev->tail;
                pList->prev->tail->next = newNode;
            } else {
                if(newNode != unusedNodeTail){
                    newNode->prev = unusedNodeTail;
                    unusedNodeTail->next = newNode;
                }
            }

            newNode->next = pList->head;

            pList->head = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            return LIST_SUCCESS;
        } else if (pList->oobStatus == LIST_OOB_END) {
            Node *newNode = Node_create();
            pList->itemCount++;
            pList->oobStatus = -1;
            newNode->item = pItem;

            if (pList->tail != newNode->next){
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;
                pList->tail->next = newNode;
            }

            if(pList->next != NULL && pList->next->head != NULL){
                newNode->next = pList->next->head;
                pList->next->head->prev = newNode;
            } else {
                if(newNode != unusedNodeHead){
                    newNode->next = unusedNodeHead;
                    unusedNodeHead->prev = newNode;
                }
            }

            newNode->prev = pList->tail;

            pList->tail = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            return LIST_SUCCESS;
        } else {
            Node *newNode = Node_create();
            pList->itemCount++;
            newNode->item = pItem; 

            if (pList->current->prev != newNode) {
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;

                newNode->next = pList->current;
                newNode->prev = pList->current->prev;
            }            

            if (pList->current == pList->head) {
                if (pList->prev != NULL && pList->prev->tail != NULL) {
                    newNode->prev = pList->prev->tail;
                    pList->prev->tail->next = newNode;
                } else {
                    if(newNode != unusedNodeTail){
                        newNode->prev = unusedNodeTail;
                        unusedNodeTail->next = newNode;
                    }
                }
                pList->head = newNode;
            }

            pList->current->prev->next = newNode;
            pList->current->prev = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            return LIST_SUCCESS;
        }
    }

    return LIST_FAIL;
}

// Add new item to end of list
int List_append(List* pList, void* pItem){
    if (usedNodeCount < LIST_MAX_NUM_NODES) {
        if(pList->current == pList->head && pList->current == pList->tail && pList->current->item == NULL){
            pList->current->item = pItem;
            pList->currentItem = pItem;
            pList->itemCount++;

            return LIST_SUCCESS;
        } else {
            Node *newNode = Node_create();
            pList->itemCount++;
            pList->oobStatus = -1;
            newNode->item = pItem;

            if (pList->tail != newNode->prev){
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;
                pList->tail->next = newNode;
            }

            if(pList->next != NULL && pList->next->head != NULL){
                newNode->next = pList->next->head;
                pList->next->head->prev = newNode;
            } else {
                if (newNode != unusedNodeHead) {
                    newNode->next = unusedNodeHead;
                    unusedNodeHead->prev = newNode;
                }
            }

            newNode->prev = pList->tail;

            pList->tail = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;
            
            if (pList->oobStatus == LIST_OOB_START || pList->oobStatus == LIST_OOB_END){
                pList->oobStatus = -1;
            }
            
            return LIST_SUCCESS;
        }
    }

    return LIST_FAIL;
}

// Add new item to front of list
int List_prepend(List* pList, void* pItem){
    if (usedNodeCount < LIST_MAX_NUM_NODES) {
        if(pList->current == pList->head && pList->current == pList->tail && pList->current->item == NULL){
            pList->current->item = pItem;
            pList->currentItem = pItem;
            pList->itemCount++;

            return LIST_SUCCESS;
        } else {
            Node* newNode = Node_create();
            pList->itemCount++;
            newNode->item = pItem;

            if (pList->head != newNode->next){
                Node* nodeBefore = newNode->prev;
                Node* nodeAfter = newNode->next;

                nodeBefore->next = newNode->next;
                nodeAfter->prev = newNode->prev;
                pList->head->prev = newNode;
            }     

            if (pList->prev != NULL && pList->prev->tail != NULL) {
                pList->prev->tail->next = newNode;
                newNode->prev = pList->prev->tail;
            } else {
                if (newNode != unusedNodeTail) {
                    newNode->prev = unusedNodeTail;
                    unusedNodeTail->next = newNode;
                }
            }

            newNode->next = pList->head;

            pList->head = newNode;
            pList->current = newNode;
            pList->currentItem = pItem;

            if (pList->oobStatus == LIST_OOB_START || pList->oobStatus == LIST_OOB_END){
                pList->oobStatus = -1;
            }

            return LIST_SUCCESS;
        }
    }

    return LIST_FAIL;
}

// Removes current node
void* List_remove(List* pList){
    if(pList->oobStatus == LIST_OOB_START || pList->oobStatus == LIST_OOB_END || pList->currentItem == NULL){
        return NULL;
    }
    
    Node* removedNode = pList->current;
    void* removedItem = pList->currentItem;
    Node* nodeBefore = removedNode->prev;
    Node* nodeAfter = removedNode->next;

    pList->itemCount--;

    if(pList->current == pList->head){ // Moves bottom/head spot forward by 1
        pList->current = pList->current->next;
        pList->head = pList->current;
        pList->currentItem = pList->current->item;
    } else if (pList->current == pList->tail) { // Moves top/tail spot back by 1
        pList->current = pList->current->prev;
        pList->tail = pList->current;
        pList->currentItem = pList->current->item;
    } else { // Moves current spot forward if middle of list
        pList->current = pList->current->next;
        pList->currentItem = pList->current->item;
    }

    nodeBefore->next = nodeAfter;
    nodeAfter->prev = nodeBefore;
    removedNode->item = NULL;
    removedNode->next = unusedNodeTail->next;
    removedNode->prev = unusedNodeTail;
    unusedNodeTail->next->prev = removedNode;
    unusedNodeTail->next = removedNode;
    unusedNodeTail = removedNode;
    usedNodeCount--;
    
    return removedItem;
}

// Remove tail and make node previous to tail the new tail
void* List_trim(List* pList){
    if(pList->head != pList->tail){
        pList->current = pList->tail;

        List_remove(pList);

        pList->currentItem = pList->tail->item;
        return pList->currentItem;
    }

    return NULL;
}

// Add list 2 to end of list 1. Delete list 2.
void List_concat(List* pList1, List* pList2){
    if(pList1->head != NULL && pList2->head != NULL && pList1->tail != NULL && pList2->tail != NULL){
        pList1->tail->next = pList2->head;
        pList2->head->prev = pList1->tail;
        pList1->tail = pList2->tail;

        pList1->itemCount += pList2->itemCount;

        List_delete(pList2);
    }
}

// Deletes list and makes used nodes/head available
void List_free(List* pList, FREE_FN pItemFreeFn){
    // Handles Nodes
    Node *node = pList->head;

    if (usedNodeCount == LIST_MAX_NUM_NODES) {
        unusedNodeHead = pList->head;
    }

    if (pList->itemCount > 0){
        while (pList->itemCount > 0) {
            Node *nextNode = NULL; // Makes empty node that will load next spot in list. (Doesn't set current node to be completely empty)

            // Assigns next node if list isn't a single node or empty
            if(pList->tail != pList->head)
                nextNode = node->next; 

            // If node isn't empty move location
            if (node->item != NULL){ 
                pItemFreeFn(node->item); // Frees item w/ user function
                node->item = NULL;

                // Node in front deleted node
                node->next->prev = node->prev;

                // Node behind deleted node
                node->prev->next = node->next;

                // Nodes around new spot at end of list
                unusedNodeTail->next->prev = node;
                unusedNodeTail->next = node;

                // Move deleted node spot
                node->next = unusedNodeTail->next;
                node->prev = unusedNodeTail;
                unusedNodeTail = node;

                usedNodeCount--;
            }
            
            pList->itemCount--;
            node = nextNode;
        }
    }

    List_delete(pList);
}

// Utility function for handling list deletion
static void List_delete(List* pList){
    // Handles lists
    List* beforeList = pList->prev;
    List* afterList = pList->next;

    if (pList->next != NULL && beforeList != NULL)
        beforeList->next = pList->next;

    afterList->prev = pList->prev;

    pList->head = NULL;
    pList->tail = NULL;
    pList->current = NULL;
    pList->currentItem = NULL;
    pList->next = NULL;
    pList->prev = unusedListTail;
    pList->itemCount = 0;

    unusedListTail->next = pList;

    pList = unusedListTail;

    usedListCount--;
}

// Searches selected list using comparator
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    Node* node = pList->current;

    // Loop from current item until after the tail
    while (node != pList->tail && node != NULL) {
        // If search matches the item looked for
        if((*pComparator)(pList->current->item, pComparisonArg)){
            pList->current = node;
            pList->currentItem = node->item;

            return pList->current->item;
        }

        node = node->next;
        pList->current = node->item;
    }

    pList->oobStatus = LIST_OOB_END;
    pList->currentItem = NULL;
    return NULL;
}