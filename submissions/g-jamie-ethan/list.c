#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "list.h"

//static arrays to store variables.
static List lists[LIST_MAX_NUM_HEADS];
static Node nodes[LIST_MAX_NUM_NODES];
static int listIndexes[LIST_MAX_NUM_HEADS];
static int nodeIndexes[LIST_MAX_NUM_NODES];
static int listIndexHead = 0;
static int nodeIndexHead = 0;
static bool isFirst = false;
static bool isBeforeFront = false;
static bool isAfterBack = false;

//helper function to make new node pointers.
Node* createNode(void* contents) {
    if(nodeIndexHead >= LIST_MAX_NUM_NODES) {
        //condition is false; no more memory left!
        return (Node *) NULL;
    }
    //create a new node pointer stored at the index of the head
    Node* newNode = &(nodes[nodeIndexes[nodeIndexHead]]);

    //assign the attributes of the node accordingly
    newNode->nodeNumber = nodeIndexes[nodeIndexHead];
    newNode->contents = contents;
    newNode->previousElement = NULL;
    newNode->nextElement = NULL;

    nodeIndexHead++;
    return newNode;
}

//helper functions used to help debug.
int nodeNumber(Node* nd) {
    return nd->nodeNumber;
}

int getIndexHead() {
    return listIndexHead;
}

int listNumber(List* list) {
    return list->listIndex;
}

List* List_create() {
    //ONLY on first iteration of List_create, we populate the index
    //arrays with numbers.
    if (!isFirst) {
        isFirst = true;
        for (int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
            listIndexes[i] = i;
        }
        for (int i = 0; i < LIST_MAX_NUM_NODES; i++) {
            nodeIndexes[i] = i;
        }
    }
    
    if (listIndexHead >= LIST_MAX_NUM_HEADS) {
        return NULL;
    }

    //initialize a new list pointer to a location in the static array.
    List* newList = &lists[listIndexes[listIndexHead]];

    // initialize the nodes stored in List object which store the front, back, and current Nodes.
    newList->front = NULL;
    newList->back = NULL;
    newList->current = NULL;

    //store the list number in the List object
    newList->listIndex = listIndexes[listIndexHead];

    //initialize the listSize
    newList->listSize = 0;

    //increment the listIndexHead for the next list to be created.
    listIndexHead++;
    return newList;
}

int List_count(List* pList) {
    return pList->listSize;
}

void* List_first(List* pList) {
    if (pList->listSize == 0) {
        pList->current = NULL;
        return NULL;
    }
    //reset the boolean values to false that indicate whether the current pointer
    //is before the front or after the back of the array.
    isBeforeFront = false;
    isAfterBack = false;

    pList->current = pList->front;
    return pList->front->contents;
}

 void* List_last(List* pList) {
    if (pList->listSize == 0) {
        pList->current = NULL;
        return NULL;
    }
    //reset the boolean values to false that indicate whether the current pointer
    //is before the front or after the back of the array.
    isBeforeFront = false;
    isAfterBack = false;

    pList->current = pList->back;
    return pList->back->contents;
 }

 void* List_next(List* pList){
    if (pList->listSize == 0 || isAfterBack) {
        return NULL;
    } else if (isBeforeFront) {
        isBeforeFront = false;
        pList->current = pList->front;
        return pList->current->contents;
    } else if (pList->current->nextElement == NULL) {
        //current is set beyond the end of the list
        isAfterBack = true;
        isBeforeFront = false;
        pList->current = NULL;
        return NULL;
    } else {
        pList->current = pList->current->nextElement;
        return pList->current->contents;
    }
 }

void* List_prev(List* pList) {
    if (pList->listSize == 0 || isBeforeFront) {
        return NULL;
    } else if (isAfterBack) {
        isAfterBack = false;
        pList->current = pList->back;
        return pList->current->contents;
    } else if (pList->current->previousElement == NULL) {
        //current is set before the front of the list
        isBeforeFront = true;
        isAfterBack = false;
        pList->current = NULL;
        return NULL;
    } else {
        pList->current = pList->current->previousElement;
        return pList->current->contents;
    }
}

void* List_curr(List* pList) {
    if (pList->listSize == 0) {
        return NULL;
    } else if (pList->current == NULL) {
        return NULL;
    } else {
        return pList->current->contents;
    }
}

int List_insert_after(List* pList, void* pItem) {
    if (nodeIndexHead >= LIST_MAX_NUM_NODES) {
        return -1;
    }
    // if list is empty or the current pointer is at the back of the list, simply append.
    if (pList->listSize == 0 || pList->current == pList->back) {
        List_append(pList, pItem);
        return 0;
    }
    // if pointer is before start of list, prepend.
    else if (pList->current == NULL && isBeforeFront) {
        isBeforeFront = false;
        isAfterBack = false;
        List_prepend(pList, pItem);
        return 0;
    }
    // if pointer is after start of list, append.
    else if (pList->current == NULL && isAfterBack) {
        isAfterBack = false;
        isBeforeFront = false;
        List_append(pList, pItem);
        return 0;
    }
    //if pointer is anywhere in middle of list, just add in.
    else {
        Node* nd = createNode(pItem);
        //set nd's contents, previous and next elements accordingly
        nd->previousElement = pList->current;
        nd->nextElement = pList->current->nextElement;
        nd->contents = pItem;

        //set current's next element pointer to nd
        pList->current->nextElement->previousElement = nd;
        pList->current->nextElement = nd;
        pList->current = nd;

        pList->listSize++;
        return 0;
    }
    return -1;
}

int List_insert_before(List* pList, void* pItem) {
    if (nodeIndexHead >= LIST_MAX_NUM_NODES) {
        return -1;
    }
    // if list is empty
    if (pList->listSize == 0) {
        List_append(pList, pItem);
        return 0;
    }
    // if pointer is at front of list, simply prepend.
    else if (pList->current == pList->front) {
        List_prepend(pList, pItem);
        return 0;
    }
    // if pointer is before start of list, prepend.
    else if (pList->current == NULL && isBeforeFront) {
        isBeforeFront = false;
        List_prepend(pList, pItem);
        return 0;
    }
    // if pointer is after start of list, append.
    else if (pList->current == NULL && isAfterBack) {
        isAfterBack = false;
        List_append(pList, pItem);
        return 0;
    }
    //if pointer is anywhere in middle of list, just add in.
    else {
        Node* nd = createNode(pItem);
        //set nd's contents, previous and next elements accordingly
        nd->nextElement = pList->current;
        nd->previousElement = pList->current->previousElement;
        nd->contents = pItem;

        //set current's next element pointer to nd
        pList->current->previousElement->nextElement = nd;
        pList->current->previousElement = nd;
        pList->current = nd;

        pList->listSize++;
        return 0;
    }
    return -1;
}

int List_append(List* pList, void* pItem) {
    if (nodeIndexHead >= LIST_MAX_NUM_NODES) {
        return -1;
    } else if (pList->listSize == 0) {
        //create a new node and set all of plist's node attributes to its pointer.
        Node* nd = createNode(pItem);
        pList->front = nd;
        pList->back = nd;
        pList->current = nd;
        pList->listSize++;
        isBeforeFront = false;
        isAfterBack = false;
    } else {
        Node* nd = createNode(pItem);

        //set the previous node of the new node to the last element of the (old) array.
        nd->previousElement = pList->back;
        nd->nextElement = NULL;

        //set the new current node to the added item
        pList->current = nd;
        isBeforeFront = false;
        isAfterBack = false;

        //set the next element of the (old) back of the list to the new node.
        pList->back->nextElement = nd;

        //set the back of the list to the new node.
        pList->back = nd;
        pList->listSize++;
    }
    return 0;
}

int List_prepend(List* pList, void* pItem) {
    if (nodeIndexHead >= LIST_MAX_NUM_NODES) {
        return -1;
    }
    if (pList->listSize == 0) {
        //create a new node and set all of plist's node attributes to its pointer.
        Node* nd = createNode(pItem);
        pList->front = nd;
        pList->back = nd;
        pList->current = nd;
        pList->listSize++;
        isBeforeFront = false;
        isAfterBack = false;
    } else if (pList->listSize == 1) {
        Node* nd = createNode(pItem);
        pList->back = pList->front;
        pList->back->previousElement = nd;
        pList->front = nd;
        pList->current = nd;
        nd->nextElement = pList->back;
        pList->listSize++;
    } else {
        Node* nd = createNode(pItem);

        //set the next element of the new node to the (old) front of the list.
        nd->nextElement = pList->front;
        nd->previousElement = NULL;

        //set the previous element of the (old) front of the list to the new node.
        pList->front->previousElement = nd;

        //set the front and current of the list to the new node
        pList->front = nd;
        pList->current = nd;
        pList->listSize++;
    }
    return 0;
}

void* List_remove(List* pList) {
    //if the current pointer isnt within the list, or if the list is empty, return NULL.
    if (isBeforeFront || isAfterBack || pList->listSize == 0) {
        return NULL;
    } 
    //if the current element is equal to the back, simply trim using List_trim().
    else if (pList->current == pList->back) {
        return List_trim(pList);
    } 
    //if the current element is the front, it is a special case that only requires the reassignment
    //of current's next element.
    else if (pList->current == pList->front) {
        // store the return value of the removed node in a temporary variable
        void* returnVal = pList->current->contents;

        //decrement the variable managing the next available index available, and set it
        //to the newly removed node.
        nodeIndexHead--;
        nodeIndexes[nodeIndexHead] = pList->front->nodeNumber;

        //set the new front to front's new element, and its previous one to NULL (since its the front)
        pList->front = pList->front->nextElement;
        pList->front->previousElement = NULL;

        //set current pointer to front, and then decrement list size.
        pList->current = pList->front;
        pList->listSize--;
        return returnVal;
    } else {
        void* returnVal = pList->current->contents;
        //decrement the head of the node index array by 1
        nodeIndexHead--;

        //set the next available index (at nodeIndexes[nodeIndexHead]) to whatever the index was in the back of the list.
        nodeIndexes[nodeIndexHead] = pList->current->nodeNumber;

        //set the previous node's next element to current's next element.
        pList->current->previousElement->nextElement = pList->current->nextElement;
        pList->current->nextElement->previousElement = pList->current->previousElement;
        pList->current->contents = NULL;
        pList->current = pList->current->nextElement;
        
        pList->listSize--;
        return returnVal;
    }
}

void* List_trim(List* pList) {
    if (pList->listSize == 0) {
        return NULL;
    } else if (pList->listSize == 1) {
        void* returnVal = pList->current->contents;
        nodeIndexHead--;
        nodeIndexes[nodeIndexHead] = pList->back->nodeNumber;
        pList->back = NULL;
        pList->front = NULL;
        pList->current = NULL;
        pList->listSize--;
        return returnVal;
    } else {
        void* returnVal = pList->back->contents;
        //decrement the head of the node index array by 1
        nodeIndexHead--;

        //set the next available index (at nodeIndexes[nodeIndexHead]) to whatever the index was in the back of the list.
        nodeIndexes[nodeIndexHead] = pList->back->nodeNumber;

        //set the new back of the list to the element before
        pList->back = pList->back->previousElement; 

        //since the back doesn't have a next element, set it to null.
        pList->back->nextElement = NULL;

        //make the new back the current element
        pList->current = pList->back;

        //decrement list size
        pList->listSize--;

        //return the contents of the trimmed node.
        return returnVal;
    }
}  

void List_concat(List* pList1, List* pList2) {
    // if the second list is empty, simply don't do anything.
    if (pList2->listSize == 0) {
        return;
    } 
    // if list1 is empty, transfer all the data from list2 into list1, and "free" the list.
    else if (pList1->listSize == 0) {
        pList1->front = pList2->front;
        pList1->back = pList2->back;
        pList1->current = pList2->current;
        pList1->listIndex = pList2->listIndex;
        pList1->listSize = pList2->listSize;
        listIndexHead--;
        listIndexes[listIndexHead] = pList2->listIndex;
    } else {
        //connect the end of list one to the front of list 2
        pList1->back->nextElement = pList2->front;
        pList2->front->previousElement = pList1->back;

        //set the new back of the list to the back of pList2
        pList1->back = pList2->back;

        //combine the two size variables.
        pList1->listSize += pList2->listSize;

        //"free" pList2
        listIndexHead--;
        listIndexes[listIndexHead] = pList2->listIndex;
    }
}

void List_free(List* pList, FREE_FN pItemFreeFn) {
    // set current to first item of the list
    List_first(pList);
    //loop to free each element of the list using the function pointer (defined in main.c)
    while(pList->current != NULL) {
        (*pItemFreeFn)(pList->current->contents);
        nodeIndexHead--;
        nodeIndexes[nodeIndexHead] = pList->current->nodeNumber;
        List_next(pList);
    }
    //freeing the list index to be available for future use, and resetting its attributes.
    listIndexHead--;
    listIndexes[listIndexHead] = pList->listIndex;
    pList->listSize = 0;
    pList->front = NULL;
    pList->back = NULL;
    pList->current = NULL;
}

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    //if the current pointer is before the front, set it to the first element of the list and
    //set the boolean indicator to false.
    if (isBeforeFront) {
        List_first(pList);
        isBeforeFront = false;
    }
    //if the current pointer is after the end of the list, return null by default.
    if (isAfterBack) {
        return NULL;
    }
    //loop that uses the helper function to figure out if the argument exists in the list.
    while(pList->current != NULL) {
        bool isMatch = (*pComparator)(pComparisonArg, pList->current->contents);
        if (isMatch) {
            return pList->current->contents;
        } else {
            List_next(pList);
        }
    }
    //if the loop is passed through, set the boolean indicating the current pointer
    //is past end of the list, and return NULL.
    pList->current = NULL;
    isAfterBack = true;
    return NULL;
}