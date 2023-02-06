#include "my_malloc_0.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "limits.h"



// void * ff_malloc(size_t size) {

//   // if (head == NULL){
//   //   return (void*)allocate_new_block(size) + sizeof(Metadata);
//   // }

//   Metadata* trav = head;
  
//   while (trav != NULL && trav->size < size){
//     trav = trav -> next;
//   }

//   return reuse_block(size, trav);

// }


void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  //int sbrk_lock = 0;
  //void * p = bf_malloc(size, sbrk_lock);

  void * p = bf_malloc(size);
  pthread_mutex_unlock(&lock);
  return p;
}


void ts_free_lock(void * ptr) {
  pthread_mutex_lock(&lock);
  bf_free(ptr);
  pthread_mutex_unlock(&lock);
}

// void * ts_malloc_nolock(size_t size) {
//   int sbrk_lock = 1;
//   void * p = bf_malloc(size, sbrk_lock);
//   return p;
// }

// void ts_free_nolock(void * ptr) {
//   bf_free(ptr);
// }

//void * reuse_block(size_t size, Metadata * trav, int sbrk_lock) {
void * reuse_block(size_t size, Metadata * trav) {

  if (trav == NULL){
    //return (void*)allocate_new_block(size, sbrk_lock) + sizeof(Metadata);
    return (void*)allocate_new_block(size) + sizeof(Metadata);
  }
  else{

    if (trav->size - size <= sizeof(Metadata)){
      remove_from_ll(trav);
    }
    else{
      
      remove_from_ll(trav);//next and previous set to null, but size still unchanged Metadata* p = (void*)trav + sizeof(Metadata) + size;
      
      Metadata* p = (void*)trav + sizeof(Metadata) + size;
      p->next = NULL;
      p->prev = NULL;
      p->size = trav->size - size - sizeof(Metadata);

      trav->size = size;//modified after used
      
      add_to_ll(p);
      check_adjacent(p);
  
      
    }

    return (void*)trav + sizeof(Metadata);

  }
  
}

//void * allocate_new_block(size_t size, int sbrk_lock) {
void * allocate_new_block(size_t size) {
 
  data_segment += size + sizeof(Metadata);

  //Metadata * new_block = NULL;

  //if (sbrk_lock == 0){
  Metadata * new_block = sbrk(size + sizeof(Metadata));
  //}
  // else{
    
  //   pthread_mutex_lock(&lock);
  //   Metadata * new_block = sbrk(size + sizeof(Metadata));
  //   pthread_mutex_unlock(&lock);

  // }

  new_block->size = size;
  
  new_block->prev = NULL;
  new_block->next = NULL;

  return new_block;

}

void add_to_ll(Metadata * p) {

  data_segment_free += p->size + sizeof(Metadata);

  if (head == NULL){
    head = p;
    p->next = NULL;
    p->prev = NULL;
    return;
  }

  if (p < head){
    p->next = head;
    head->prev = p;
    head = p;
    p->prev = NULL;
    return;
  }

  Metadata* trav = head;

    while (trav->next != NULL && p > trav->next){
        trav = trav->next;
    }

    if (trav->next == NULL){

        trav->next = p;
        p->prev = trav;
        p->next = NULL;

    }

    else{

        Metadata* store = trav->next;
        trav->next = p;
        p->next = store;
        p->prev = trav;
        store->prev = p;

    }

}

void remove_from_ll(Metadata * p) {

  data_segment_free -= p->size + sizeof(Metadata);

  if (p->next == NULL && p->prev == NULL) {
    head = NULL;
  }
  else if (p->next == NULL) {
    
    p->prev->next = NULL;
    p->prev = NULL;
  }
  else if (head == p) {
    head = p->next;
    head->prev = NULL;
    p->next = NULL;
  }
  else {
    p->prev->next = p->next;
    p->next->prev = p->prev;
    p->next = NULL;//set to null before leaving the list
    p->prev = NULL;
  }

  

}


void check_adjacent(Metadata* curr){

    if (curr->next != NULL && (void*) curr + sizeof(Metadata) + curr->size == (void*) curr->next){
        
        curr->size += curr->next->size + sizeof(Metadata);
        data_segment_free += curr->next->size + sizeof(Metadata);//compensate for removal later
        remove_from_ll(curr->next);
        //data_segment_free += curr->next->size + sizeof(Metadata);//core dump!
        
    }

    if (curr->prev != NULL && (void*) curr->prev + sizeof(Metadata) + curr->prev->size == (void*) curr){
        

        curr->prev->size += curr->size + sizeof(Metadata);
        data_segment_free += curr->size + sizeof(Metadata);
        remove_from_ll(curr);
        

    }

}

void ff_free(void * ptr) {

  Metadata * p = (void*)ptr - sizeof(Metadata);

  add_to_ll(p);

  check_adjacent(p);

}

//void * bf_malloc(size_t size, int sbrk_lock) {
void * bf_malloc(size_t size) {


  // if (head == NULL){
  //   return (void*)allocate_new_block(size) + sizeof(Metadata);
  // }

  Metadata* trav = head;
  Metadata* theNode = NULL;
  //printf("%zu\n", size);

  while (trav != NULL){

    if (trav->size == size){
      remove_from_ll(trav);
      return (void*)trav + sizeof(Metadata);
      
    }

    if (trav->size > size ){
      
      if (theNode == NULL){
        theNode = trav;
      }
      else{
        if (trav->size < theNode->size){
          theNode = trav;
        }
      }

    }

    trav = trav->next;

  }

  //return reuse_block(size, theNode, sbrk_lock);
  return reuse_block(size, theNode);
  

}

void bf_free(void * ptr) {
  return ff_free(ptr);
}

unsigned long get_data_segment_size() {
  return data_segment;
}

unsigned long get_data_segment_free_space_size() {
  return  data_segment_free;
}