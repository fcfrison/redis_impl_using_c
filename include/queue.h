#ifndef QUEUE_H
typedef void (*destructor_fn)(void*);
typedef struct Queue Queue;
typedef struct ListNode ListNode;
/**
 * A node in a doubly-linked list structure.
 * 
 * @member prev Pointer to the previous node in the list
 * @member next Pointer to the next node in the list
 * @member data Pointer to the node's data (can store any data type)
 */
struct ListNode{
    ListNode* prev;
    ListNode* next;
    void*     data;
};
/**
 * A queue data structure implementation using a doubly-linked list.
 * 
 * @member front        Pointer to the first node in the queue
 * @member rear         Pointer to the last node in the queue
 * @member enqueue      Function pointer to add an element to the queue
 * @member dequeue      Function pointer to remove and return the front element
 * @member get_front    Function pointer to get the front node without removing it
 * @member get_rear     Function pointer to get the rear node without removing it
 * @member delete_queue Function pointer to deallocate the entire queue
 * @member destructor   Function pointer to clean up node data when deallocating
 * 
 * @details The queue supports storing any data type through void pointers.
 * Memory management for the stored data is handled by the provided destructor
 * function when elements are removed or the queue is deleted.
 */
struct Queue{
    ListNode*     front;
    ListNode*     rear;
    void          (*enqueue)(Queue* q, void* n);
    void*         (*dequeue)(Queue* q);
    ListNode*     (*get_front)(Queue* q);
    ListNode*     (*get_rear)(Queue* q);
    void          (*delete_queue)(Queue* q);
    destructor_fn destructor;  // Function pointer to cleanup data
};

ListNode* get_front(Queue* q);
ListNode* get_rear(Queue* q);
Queue*    init_queue(void(*func)(void*));
void      enqueue(Queue* q, void* n);
void*     dequeue(Queue* q);
void      delete_queue(Queue* q);
#endif