#include <stdlib.h>
#include <stdio.h>
#include "../include/queue.h"

/**
 * Gets the front node of the queue.
 * 
 * @param q Pointer to the Queue structure
 * @return Pointer to the front ListNode, or NULL if queue is NULL
 */
void* 
get_front(Queue* q){
    if(!q || !q->front) return NULL;
    return q->front->data;
}
/**
 * Gets the rear node of the queue.
 * 
 * @param q Pointer to the Queue structure
 * @return Pointer to the rear ListNode, or NULL if queue is NULL
 */
void* 
get_rear(Queue* q){
    if(!q || !q->rear) return NULL;
    return q->rear->data;
}
/**
 * Adds a new node to the rear of the queue.
 * 
 * @param q Pointer to the Queue structure
 * @param data Pointer to the data to be stored in the new node
 * 
 * @details Creates a new ListNode with the provided data and adds it to the rear
 * of the queue. If this is the first element, both front and rear will point to it.
 * Otherwise, the new node is linked to the current rear node and becomes the new rear.
 * Does nothing if queue pointer is NULL or if memory allocation fails.
 */
void
enqueue(Queue* q, void* data){
    if(!q){
        return;
    }
    ListNode* n = (ListNode*)calloc(1,sizeof(ListNode));
    if(!n){
        return;
    }
    n->data = data;
    n->next = NULL;
    // 1st element
    if(!q->front && !q->rear){
        q->front = q->rear = n;
        n->prev  = NULL;
        return;
    }
    ListNode* prev = q->rear;
    prev->next = n;
    n->prev    = prev;
    q->rear    = n;
    return; 
};
/**
 * Removes and returns the data from the front of the queue.
 *
 * @param q Pointer to the Queue structure
 * @return Pointer to the data from the front node, or NULL if queue is empty or NULL
 *
 * @details Handles three cases:
 * 1. Queue is NULL or empty: returns NULL
 * 2. Queue has one element: sets front and rear to NULL
 * 3. Queue has multiple elements: updates front pointer to next node
 * In all successful cases, frees the removed node and returns its data.
 */
void*
dequeue(Queue* q){
    void* data = NULL;
    if(!q || (!q->front)){
        return data;
    }
    ListNode *front, *next;
    if(q->front==q->rear){
        front    = q->front;
        q->front = NULL;
        q->rear  = NULL;
        data = front->data;
        free(front);
        return data;
    }
    front = q->front;
    next  = front->next;
    front->next = NULL;
    next->prev  = NULL;
    q->front    = next;
    data = front->data;
    free(front);
    return data;
};
/**
 * Initializes a new Queue structure with function pointers and destructor.
 *
 * @param func Function pointer to the destructor for queue elements
 * @return Pointer to the newly created Queue structure, or NULL if allocation fails
 *
 * @details Allocates memory for a new Queue structure and initializes all its
 * function pointers (dequeue, enqueue, get_front, get_rear, delete_queue) and
 * the destructor function. Front and rear pointers are initialized to NULL.
 */
Queue* 
init_queue(void(*func)(void*)){
    Queue* q = (Queue*)calloc(1,sizeof(Queue));
    if(!q){
        return NULL;
    }
    q->front     = NULL;
    q->rear      = NULL;
    q->dequeue   = &dequeue;
    q->enqueue   = &enqueue;
    q->get_front = &get_front;
    q->get_rear  = &get_rear;
    q->delete_queue = &delete_queue;
    q->destructor = func;
    return q;
}
/**
 * Deallocates all memory associated with the queue.
 *
 * @param q Pointer to the Queue structure
 *
 * @details Iterates through all nodes in the queue, calling the destructor
 * function (if provided) on each node's data, then frees the node itself.
 * Finally, frees the Queue structure. If q is NULL, function returns without
 * doing anything. The destructor is only called on non-NULL data when a
 * destructor function has been provided during queue initialization.
 */
void
delete_queue(Queue* q){
    if(!q){
        return;
    }
    ListNode *node, *next;
    for (node = q->front; node; node = next){
        next = node->next;
        if(node->data && q->destructor){
            q->destructor(node->data);
        }
        free(node);
    }
    free(q);
    return;
}