#include "list.h"
#include "stdio.h"
#include "assert.h"

static struct Stack{
    List * current;
    List * head;
    int size;
};

static Node nodePool[LIST_MAX_NUM_NODES];
static List listPool[LIST_MAX_NUM_HEADS];
static List recycleNodeList;
static struct Stack recycleStack;
static int arrayOfListIndex = 0;
static int arrayOfNodeIndex = 0;
static enum ListOutOfBounds oobStartEnum = LIST_OOB_START;
static enum ListOutOfBounds oobEndEnum = LIST_OOB_END;


static void assertNull(List * pList){
    assert(pList != NULL);
}

static void setUpRecycleStack(struct Stack * pStack){
    pStack->current = NULL;
    pStack->head = NULL;
    pStack->size = 0;
}

static List * pop(struct Stack * pStack){

    if(pStack->size == 0){
        return NULL;
    }

    List * saveCurrent = pStack->current;

    if(pStack->size > 1)
    {
        saveCurrent->down->up = NULL;
    }

    pStack->current = pStack->current->down;

    pStack->size--;

    return saveCurrent;
}

static void push(struct Stack * pStack, List * listPtr){
    if(pStack->size == 0){
        pStack->current = listPtr;
        pStack->head = listPtr;
    }

    else{
        listPtr->down = pStack->current;
        pStack->current->up = listPtr;

        pStack->current = listPtr;
    }

    pStack->size++;
}

static void setUpList(List * pList){
    
    Node oobStart;

    oobStart.item = &oobStartEnum;
    oobStart.prev = NULL;
    oobStart.next = NULL;

    Node oobEnd;

    oobEnd.item = &oobEndEnum;
    oobEnd.prev = NULL;
    oobEnd.next = NULL;
    
    pList->oobStart = oobStart;
    pList->oobEnd = oobEnd;

    pList->tail = NULL;
    pList->current = NULL;
    pList->head = NULL;
    pList->size = 0;
    pList->up = NULL;
    pList->down = NULL;
}

static List * createNewList(){
    List * newList = &listPool[arrayOfListIndex];
    setUpList(newList);
    return newList;
}

static void resetList(List * pList){
    setUpList(pList);
}
static void resetNode(Node * nodePtr){
    nodePtr->next = NULL;
    nodePtr->prev = NULL;
    nodePtr->item = NULL;
}

static Node * createNode(){
    Node newNode;

    newNode.next = NULL; 
    newNode.prev = NULL;
    newNode.item = NULL;

    nodePool[arrayOfNodeIndex] = newNode;

    return &nodePool[arrayOfNodeIndex++];

}

//Test after delete has been implemented
List* List_create(){
    List * availableList = NULL;
    if(arrayOfListIndex == 0){
        setUpList(&recycleNodeList);
        setUpRecycleStack(&recycleStack);
    }

    if(recycleStack.size > 0){
        availableList = recycleStack.current;
        pop(&recycleStack);  
    }
    else
    {
        if(arrayOfListIndex >= LIST_MAX_NUM_HEADS){
            return NULL;
        }

        availableList = createNewList();
        arrayOfListIndex++;
    }

    return availableList;

}

int List_count(List* pList){
    assertNull(pList);

    return pList->size;
}

static void* List_end(List * pList, Node * endPtr){
    if(pList->size == 0){
        pList->current = NULL;
        return NULL;
    }

    pList->current = endPtr;

    return pList->current->item;
}

void* List_first(List* pList){
    assertNull(pList);

    return List_end(pList, pList->head);
}

void* List_last(List* pList){
    assertNull(pList);

    return List_end(pList, pList->tail);
}

static bool isCurrPtrOutOfBoundStart(List * pList){
    return pList->current == &(pList->oobStart);
}

static bool isCurrPtrOutOfBoundEnd(List * pList){
    return pList->current == &(pList->oobEnd);
}

static bool isNextInvalid(List * pList){
    return pList->size == 0 || isCurrPtrOutOfBoundEnd(pList);
}

void* List_next(List* pList){
    assertNull(pList);

    if(isNextInvalid(pList)){
        return NULL;
    }

    pList->current = pList->current->next;

    if(isCurrPtrOutOfBoundEnd(pList)){
        return NULL;
    }

    return pList->current->item;
}

static bool isPrevInvalid(List * pList){
    return pList->size == 0 || isCurrPtrOutOfBoundStart(pList);
}

void* List_prev(List* pList){
    assertNull(pList);

    if(isPrevInvalid(pList)){
        return NULL;
    }

    pList->current = pList->current->prev;
    if(isCurrPtrOutOfBoundStart(pList)){
        return NULL;
    }

    return pList->current->item;
}

void* List_curr(List* pList){
    assertNull(pList);

    if(pList->current == NULL){
        return NULL;
    }

    return pList->current->item;
}

static void nextRecycleNode(){
    if(recycleNodeList.size == 0){
        return;
    }

    recycleNodeList.current->prev->next = NULL;


    recycleNodeList.current = recycleNodeList.current->prev;

    recycleNodeList.size--;
}

static Node * recycleNode(){
    Node * nodePtr = NULL;

    if(recycleNodeList.size > 0){
        nodePtr = recycleNodeList.current;
        nextRecycleNode();
    }
    else{
        if(arrayOfNodeIndex >= LIST_MAX_NUM_NODES){
            return NULL;
        }
        else{
            nodePtr = createNode();
        }
    }
    

    return nodePtr;
}

static void insertWhenEmpty(List* pList, Node * nodePtr, void* pItem){
    nodePtr->item = pItem;

    pList->head = nodePtr;

    pList->head->prev = &(pList->oobStart);
    (pList->oobStart).next = pList->head;

    pList->head->next = &(pList->oobEnd);
    (pList->oobEnd).prev = pList->head;

    pList->tail = pList->head;

    pList->size++;
}

static void append(List* pList, Node * nodePtr, void* pItem){

    nodePtr->item = pItem;

    nodePtr->next = pList->tail->next;
    pList->tail->next->prev = nodePtr;

    pList->tail->next = nodePtr;
    nodePtr->prev = pList->tail;

    pList->tail = nodePtr;

    pList->size++;
}

static void prepend(List* pList, Node * nodePtr, void* pItem){

    nodePtr->item = pItem;

    Node * head = pList->head;
    head->prev->next = nodePtr;
    nodePtr->prev = head->prev;

    nodePtr->next = head;
    head->prev = nodePtr;

    pList->head = nodePtr;

    pList->size++;
}

static void insertAfter(List* pList, Node * nodePtr, void* pItem){


    nodePtr->item = pItem;

    Node * current = pList->current;
    nodePtr->next = current->next;
    current->next->prev = nodePtr;

    nodePtr->prev = current;
    current->next = nodePtr;

    pList->size++;
    
}

int List_insert_after(List* pList, void* pItem){
    assertNull(pList);

    Node * newNodePtr = recycleNode(pList);

    if(newNodePtr == NULL){
        return -1;
    }

    if(pList->size == 0){
        insertWhenEmpty(pList, newNodePtr, pItem);
    }

    else if(isCurrPtrOutOfBoundStart(pList)){
        prepend(pList, newNodePtr, pItem);
    }

    else if(isCurrPtrOutOfBoundEnd(pList)){
        append(pList, newNodePtr, pItem);
    }

    else{
        insertAfter(pList, newNodePtr, pItem);

        if(pList->current == pList->tail){
            pList->tail = newNodePtr;
        }
    }

    pList->current = newNodePtr;

    return 0;

}

int List_insert_before(List* pList, void* pItem){
    assertNull(pList);

    Node * newNodePtr = recycleNode(pList);

    if(newNodePtr == NULL){
        return -1;
    }

    if(pList->size == 0){
        insertWhenEmpty(pList, newNodePtr, pItem);
    }

    else if(isCurrPtrOutOfBoundStart(pList)){
        prepend(pList, newNodePtr, pItem); 
    }

    else if(isCurrPtrOutOfBoundEnd(pList)){
        append(pList, newNodePtr, pItem);
    }

    else{

        if(pList->current == pList->head){
            pList->head = newNodePtr;
        }

        pList->current = pList->current->prev;
        insertAfter(pList, newNodePtr, pItem);
    }

    pList->current = newNodePtr;

    return 0;

}

int List_append(List* pList, void* pItem){
    assertNull(pList);

    Node * newNodePtr = recycleNode(pList);

    if(newNodePtr == NULL){
        return -1;
    }

    if(pList->size == 0){
        insertWhenEmpty(pList, newNodePtr, pItem);
    }
    else{
        append(pList, newNodePtr, pItem);
    }

    pList->current = newNodePtr;

    return 0;

}

int List_prepend(List* pList, void* pItem){
    assertNull(pList);

    Node * newNodePtr = recycleNode(pList);

    if(newNodePtr == NULL){
        return -1;
    }

    if(pList->size == 0){
        insertWhenEmpty(pList, newNodePtr, pItem);
    }
    else{
        prepend(pList, newNodePtr, pItem);
    }

    pList->current = newNodePtr;

    return 0;
}

static bool isRemoveInvalid(List * pList){
    return pList->size == 0 || isCurrPtrOutOfBoundStart(pList) || isCurrPtrOutOfBoundEnd(pList);
}

static void removeNode(List * pList){
    Node * current = pList->current;

    current->prev->next = current->next;
    current->next->prev = current->prev;

    pList->size--;
}

static void addNodeToRecycleNodeList(Node * nodePtr){

    if(recycleNodeList.size == 0){
        insertWhenEmpty(&recycleNodeList, nodePtr, NULL);
    }
    else{
         append(&recycleNodeList, nodePtr, NULL);
    }

    recycleNodeList.current = nodePtr;
}

static void resuseNode(Node * nodePtr){
    resetNode(nodePtr);
    addNodeToRecycleNodeList(nodePtr);
}

static void resetHead(List * pList){
    Node * head = pList->head;
    if(head->next == &(pList->oobEnd)){
        pList->head = NULL;
    }
    else{
        pList->head = pList->head->next;
    }
}

static void resetTail(List * pList){
    Node * tail = pList->tail;
    if(tail->prev == &(pList->oobStart)){
        pList->tail = NULL;
    }
    else{
        pList->tail = pList->tail->prev;
    }
}

void* List_remove(List* pList){
    assertNull(pList);

    if(isRemoveInvalid(pList)){
        return NULL;
    }

    Node * saveCurrent = pList->current;
    void * saveItem = saveCurrent->item;

    removeNode(pList);

    if(pList->head == saveCurrent){
        resetHead(pList);
    }
    if(pList->tail == saveCurrent){
        resetTail(pList);
    }

    pList->current = pList->current->next;
    resuseNode(saveCurrent);
    

    return saveItem;
}

void* List_trim(List* pList){
    assertNull(pList);

    if(pList->size == 0){
        return NULL;
    }

    Node * saveTail = pList->tail;
    void * saveItem = saveTail->item;

    pList->current = pList->tail;
    removeNode(pList);

    if(pList->head == saveTail){
        resetHead(pList);
    }
    
    resetTail(pList);

    pList->current = pList->tail;
    
    resuseNode(saveTail);

    return saveItem;

}


void List_concat(List* pList1, List* pList2){
    assertNull(pList1);
    assertNull(pList2);

    if(pList1->size == 0 && pList2->size > 0){
        pList1->head  = pList2->head;
        pList1->tail = pList2->tail;
        pList1->current = pList2->current;
        pList1->size = pList2->size;
        pList1->oobStart = pList2->oobStart;
        pList1->oobEnd = pList2->oobEnd;
    }

    else if(pList1->size > 0 && pList2->size > 0){
        pList1->tail->next = pList2->head;
        pList2->head->prev = pList1->tail;

        pList1->oobEnd = pList2->oobEnd;
        pList1->tail = pList2->tail;

        pList1->size+=pList2->size;
    }

    resetList(pList2);

    push(&recycleStack, pList2);
}

void List_free(List* pList, FREE_FN pItemFreeFn){
    assertNull(pList);

    List_first(pList);

    while(pList->current != NULL){
        if(isCurrPtrOutOfBoundEnd(pList)){
            break;
        }

        (*pItemFreeFn)(pList->current->item);

        List_remove(pList);

    }

    resetList(pList);

    push(&recycleStack, pList);

}

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    assertNull(pList);

    if(pList->size == 0){
        return NULL;
    }

    while(pList->current != &(pList->oobEnd)){
        if(pComparator(pList->current->item, pComparisonArg) == 1){
            return pList->current->item;
        }

        List_next(pList);
    }

    return NULL;

}