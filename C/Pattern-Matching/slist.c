#include "slist.h"
#include <stdio.h>
#include <stdlib.h>

// initializing a pointer to a list - must be allocated
void dbllist_init(dbllist_t *list) {
    if(list == NULL) {
        fprintf(stderr, "list is unallocated");
        return;
    }
    dbllist_head(list) = NULL;
    dbllist_tail(list) = NULL;
    dbllist_size(list) = 0;
}

// freeing list memory
void dbllist_destroy(dbllist_t *list, dbllist_destroy_t remove_type) {
    if(list == NULL) {
        return;
    }
    // looping through the list free ech nodes data (of specipied to do so), then freeing node
    for(dbllist_node_t *tmp = dbllist_head(list); tmp != NULL; tmp = dbllist_head(list)) {
        dbllist_head(list) = dbllist_next(dbllist_head(list));
        if(remove_type == DBLLIST_FREE_DATA && dbllist_data(tmp) != NULL) {
            free(dbllist_data(tmp));
        }
        free(tmp);
    }
}

// adding data to the end of a list
int dbllist_append(dbllist_t *list, void *data) {

    //creating new node
    dbllist_node_t* tmp = (dbllist_node_t*)malloc(sizeof(dbllist_node_t));
    if(tmp == NULL) { //malloc failed
        fprintf(stderr, "malloc failed");
        return -1;
    }
    dbllist_data(tmp) = data;
    dbllist_next(tmp) = NULL;
    dbllist_prev(tmp) = NULL;

    // empty list
    if(dbllist_head(list) == NULL) {
        dbllist_head(list) = tmp;
        dbllist_tail(list) = tmp;
    }
    else { // adding to the end
        dbllist_prev(tmp) = dbllist_tail(list);
        dbllist_next(dbllist_tail(list)) = tmp;
        dbllist_tail(list) = tmp;
    }
    dbllist_size(list) = dbllist_size(list) + 1;
    return 0;
}

// adding data to the front of a list
int dbllist_prepend(dbllist_t *list, void *data) {

    //creating new node
    dbllist_node_t* tmp = (dbllist_node_t*)malloc(sizeof(dbllist_node_t));
    if(tmp == NULL) {
        fprintf(stderr, "malloc failed");
        return -1;
    }
    dbllist_data(tmp) = data;
    dbllist_next(tmp) = NULL;
    dbllist_prev(tmp) = NULL;

    // empty list
    if(dbllist_head(list) == NULL) {
        dbllist_head(list) = tmp;
        dbllist_tail(list) = tmp;
    }
    else {
        dbllist_next(tmp) = dbllist_head(list);
        dbllist_prev(dbllist_head(list)) = tmp;
        dbllist_head(list) = tmp;
    }
    
    dbllist_size(list) = dbllist_size(list) + 1;
    return 0;
}

// removing specific node from a list
int dbllist_remove(dbllist_t *list, dbllist_node_t *to_remove, dbllist_destroy_t remove_type) {
    if(to_remove == NULL) { return 0; }

    // list connectivity issues
    if(dbllist_prev(to_remove) == NULL && dbllist_next(to_remove) == NULL) { // one node only list
        dbllist_head(list) = NULL;
        dbllist_tail(list) = NULL;
    }
    else if(dbllist_prev(to_remove) == NULL) { // list's head
        dbllist_head(list) = dbllist_next(to_remove);
        dbllist_prev(dbllist_head(list)) = NULL;
    }
    else if(dbllist_next(to_remove) == NULL) { // list's tail
        dbllist_tail(list) = dbllist_prev(to_remove);
        dbllist_next(dbllist_tail(list)) = NULL;
    }
    else {
        dbllist_next(dbllist_prev(to_remove)) = dbllist_next(to_remove);
        dbllist_prev(dbllist_next(to_remove)) = dbllist_prev(to_remove);
    }
    dbllist_size(list) = dbllist_size(list) - 1;
    

    //freeing node
    if(remove_type == DBLLIST_FREE_DATA) {
        free(dbllist_data(to_remove));
    }
    free(to_remove);
    to_remove == NULL;
    return 0;
}
