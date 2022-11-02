#include "list.h"
#include <stdio.h>
#include <assert.h>

/// allocatoing static nodes and list 
static List MylistofHeads[LIST_MAX_NUM_HEADS];
static List* pFreeHead = &MylistofHeads[0];
static Node MylistofNodes[LIST_MAX_NUM_NODES];
static Node* pFreeNode = &MylistofNodes[0];
bool Flag_uninatialze = true;

// Fuinction to create list, inatializing the array/nodes with null values.
List* List_create(){
    if (Flag_uninatialze){

        // Nodes looping and connecting to next one
        for (int i=0; i<LIST_MAX_NUM_NODES; i++){
            MylistofNodes[i].pItem = NULL;
            MylistofNodes[i].pPrevious = NULL;
            if(i+1 == LIST_MAX_NUM_NODES){
                MylistofNodes[i].pNext = NULL;
            }else{
                 MylistofNodes[i].pNext = &MylistofNodes[i+1];
            }
        }
        // ListHeads looping and connecting to next head
        for(int i=0; i<LIST_MAX_NUM_HEADS; i++){
            MylistofHeads[i].NodeCount=0;
            MylistofHeads[i].pCurrentNode = NULL;
            MylistofHeads[i].pFirstNode = NULL;
            MylistofHeads[i].pLastNode = NULL;
            if(i+1 == LIST_MAX_NUM_HEADS){
                MylistofHeads[i].pNextListHead = NULL;
            }else{
                MylistofHeads[i].pNextListHead = &MylistofHeads[i+1];
            }
            MylistofHeads[i].Reason = LIST_OOB_START;
        }
        Flag_uninatialze = false;
    }

    List* pList = pFreeHead;
    if (pList != NULL){
        pFreeHead = pFreeHead->pNextListHead;
        pList->pNextListHead = NULL;
    }
    return pList;
}

int List_count(List* pList){
    return pList->NodeCount;
}

void* List_first(List* pList){
    pList->pCurrentNode = pList->pFirstNode;
    void *pItem = pList->pFirstNode->pItem;
    return pItem;
}

void* List_last(List* pList){
    pList->pCurrentNode = pList->pLastNode;
    void* pItem = pList->pLastNode->pItem;
    return pItem;
}

void* List_next(List* pList){

    
    assert(pList->Reason == LIST_OOB_START || pList->Reason ==LIST_OOB_END);
    if(pList->pCurrentNode == pList->pLastNode){
        return pList->pCurrentNode->pItem;
    }
    if(pList->Reason == LIST_OOB_START && pList->pCurrentNode == NULL){
        pList->pCurrentNode = pList->pFirstNode;
    }
    else if(pList->Reason == LIST_OOB_END && pList->pCurrentNode == pList->pLastNode){
        //can't go forward, do Nothing 
        
    }else{
        pList->pCurrentNode = pList->pCurrentNode->pNext;
    }

    if(pList->pCurrentNode == NULL){
        pList->Reason = LIST_OOB_END;
    }

    void* pItem = pList->pCurrentNode->pItem;
    return pItem;
}

void* List_prev(List* pList){

    
    assert(pList->Reason == LIST_OOB_START || pList->Reason == LIST_OOB_END);
    if(pList->pCurrentNode == pList->pFirstNode){
        return pList->pCurrentNode->pItem;
    }
    if(pList->Reason == LIST_OOB_END && pList->pCurrentNode == NULL){
        pList->pCurrentNode = pList->pLastNode;
    }else if(pList->Reason == LIST_OOB_START && pList->pCurrentNode == NULL){
        //can't go previous, do Nothing 
    }else{
        pList->pCurrentNode = pList->pCurrentNode->pPrevious;
    }

    if(pList->pCurrentNode == NULL){
        pList->Reason = LIST_OOB_START;
    }

    void* pItem= pList->pCurrentNode->pItem;
    return pItem;
}

void* List_curr(List* pList){
   // assert(pList->pCurrentNode == NULL);
    return pList->pCurrentNode->pItem;
}

static Node* NewNode(void* pItem){
    assert(pFreeNode != NULL);
    Node* pNode = pFreeNode;
    pNode->pItem = pItem;
    pFreeNode = pFreeNode->pNext;
    pNode->pNext = NULL;
    return pNode;
}

static void AddNodeAtStart(List* pList, Node* pNode){
    pNode->pNext = pList->pFirstNode;
    pNode->pPrevious = NULL;
    
    if(pList->NodeCount <= 0){
        pList->pLastNode = pNode;
    }else{
        pList->pFirstNode->pPrevious = pNode;
    }
    pList->pFirstNode = pNode;
    pList->pCurrentNode = pNode;
    pList->NodeCount++;
}

static void AddNodeAtEnd(List* pList, Node* pNode){
    pNode->pNext = NULL;
    pNode->pPrevious = pList->pLastNode;

    if(pList->NodeCount <= 0){
        pList->pFirstNode = pNode;
    }else{
        pList->pLastNode->pNext = pNode;
    }
    pList->pLastNode = pNode;
    pList->pCurrentNode = pNode;
    pList->NodeCount++;
    
}

int List_insert_after(List* pList, void* pItem){

    if (pFreeNode == NULL || pList->NodeCount == LIST_MAX_NUM_NODES){
        return LIST_FAIL;
    }
    Node* pNode = NewNode(pItem);
    if (pList->pCurrentNode == pList->pLastNode){
       AddNodeAtEnd(pList, pNode);
    }
    else if(pList->pCurrentNode==NULL && pList->Reason == LIST_OOB_START){
        AddNodeAtStart(pList,pNode);
    }else{
        assert(pList->pCurrentNode != NULL);
        pNode->pPrevious = pList->pCurrentNode;
        pNode->pNext = pList->pCurrentNode->pNext;
        pList->pCurrentNode->pNext = pNode;
        pNode->pNext->pPrevious = pNode;
        pList->pCurrentNode = pNode;
        pList->NodeCount++;
    }
    return LIST_SUCCESS;
}

int List_insert_before(List* pList, void* pItem){
    if (pFreeNode == NULL || pList->NodeCount == LIST_MAX_NUM_NODES){
        return LIST_FAIL;
    }
    // Node* pNode = NewNode(pItem);
    List_prev(pList);
    return List_insert_after(pList,pItem);
    
    
}

int List_append(List* pList, void* pItem){
    if (pFreeNode == NULL || pList->NodeCount == LIST_MAX_NUM_NODES){
        return LIST_FAIL;
    }
    Node* pNode = NewNode(pItem);

    pList->pCurrentNode = pList->pLastNode;
    AddNodeAtEnd(pList, pNode);
    return LIST_SUCCESS;

}

int List_prepend(List* pList, void* pItem){
    if (pFreeNode == NULL || pList->NodeCount == LIST_MAX_NUM_NODES){
        return LIST_FAIL;
    }
    Node* pNode = NewNode(pItem);
    AddNodeAtStart(pList,pNode);
    return LIST_SUCCESS;
}

void* List_remove(List* pList){
    if(pList->NodeCount == 0 && pList->Reason == LIST_OOB_START){
        return NULL;
    }

    //storing the pointer of the current Node
    Node* pNodeRemove = pList->pCurrentNode;
    void* pItem = pNodeRemove->pItem;
    Node* pPreviousNode = pNodeRemove->pPrevious;
    Node* pNextNode = pNodeRemove->pNext;
    
    //linkning to new nodes if any
    if(pPreviousNode != NULL){
        pPreviousNode->pNext = pNextNode;
    }else{
        pList->pFirstNode = pNextNode;
        
    }
    
    ///linkning to new nodes if any 
    if(pNextNode!= NULL){
        pNextNode->pPrevious = pPreviousNode;
        
    }else{
        pList->pLastNode = pPreviousNode;
        
    }
    pList->NodeCount--;

    // adjusting Freenode count
    pNodeRemove->pNext = pFreeNode;
    pFreeNode = pNodeRemove;

    //resseting current to next 
    pList->pCurrentNode = pNextNode;
    if(pList->pCurrentNode == NULL){
        pList->Reason = LIST_OOB_END;
    }
    return pItem;
}

void* List_trim(List* pList){
    //removing last node
    List_last(pList);
    void* pItem = List_remove(pList);
    List_last(pList);
    
    return pItem;
}

void List_concat(List* pList1, List* pList2){

    Node* pHeadOfList2 = pList2->pFirstNode;
    Node* pTailOfList1 = pList1->pLastNode;
    //linking head and tails
    if(pTailOfList1 == NULL){
        pList1->pFirstNode = pHeadOfList2;
        pList1->pLastNode = pList2->pLastNode;
    }else{
        pTailOfList1->pNext =pHeadOfList2;
        pHeadOfList2->pPrevious = pTailOfList1;
        pList1->pLastNode = pList2->pLastNode;
    }
    // adding counts
    pList1->NodeCount += pList2->NodeCount;

    pList2->NodeCount = 0;
    pList2->pCurrentNode = NULL;
    pList2->pFirstNode = NULL;
    pList2->pLastNode = NULL;

    //Deleting the list
    List_free(pList2, NULL);
}

void List_free(List* pList, FREE_FN pItemFreeFn){
    //looping until last node
    while (List_count(pList) > 0){
        Node* pNode = List_trim(pList);

        if(pItemFreeFn != NULL){
            //cleaning memory as per notes
            (*pItemFreeFn)(pNode);
        }
    }
    pList->pNextListHead = pFreeHead;
    pFreeHead = pList;
}


void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    if(pList->pCurrentNode == NULL && pList->Reason == LIST_OOB_START){
        List_first(pList);
    }

    while(pList->pCurrentNode != NULL){
        void* pItem = pList->pCurrentNode->pItem;
        if((*pComparator)(pItem, pComparisonArg) == 1){
            return pItem;
        }
        List_next(pList);
    }

    return NULL;
}