#include "pattern_matching.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// initiallizing pm state machine - pointer must be allocated
int pm_init(pm_t *pm) {
    pm->newstate = 1;

    // create new root state
    pm->zerostate = (pm_state_t*)malloc(sizeof(pm_state_t));
    if(pm->zerostate == NULL) {
        perror("malloc failed");
        return -1;
    }
    pm->zerostate->id = 0;
    pm->zerostate->depth = 0;
    pm->zerostate->output = NULL;
    pm->zerostate->fail = NULL;

    pm->zerostate->_transitions = (dbllist_t*)malloc(sizeof(dbllist_t));
    if(pm->zerostate->_transitions == NULL) {
        perror("malloc failed");
        return -1;
    }
    dbllist_init(pm->zerostate->_transitions);

    return 0;
}

// creating pm state machine tree (black arrows)
int pm_addstring(pm_t *pm,unsigned char *pattern, _size_t n) {
    if(strlen(pattern) < 1 || n < 1) {
        printf("you tried to trick me? naughty naughty\n");
        return -1;
    }
    pm_state_t* from_state = pm->zerostate;

    // trying to progress in the tree over string
    int i;
    pm_state_t* to_state = pm->zerostate;
    for(i = 0; i < n; i++) {
        to_state = pm_goto_get(to_state, pattern[i]);
        if(to_state == NULL) {
            break;
        }
        from_state = to_state;
    }

    // adding states for each letter in the pattern
    for(; i < n; i++) {
        // creating new state
        to_state = (pm_state_t*)malloc(sizeof(pm_state_t));
        if(to_state == NULL) {
            perror("malloc failed");
            return -1;
        }
    
        // setting new state values
        to_state->id = pm->newstate++;
        to_state->depth = from_state->depth + 1;

        to_state->fail = NULL;

        to_state->output = (dbllist_t*)malloc(sizeof(dbllist_t));
        dbllist_init(to_state->output);

        to_state->_transitions = (dbllist_t*)malloc(sizeof(dbllist_t));
        dbllist_init(to_state->_transitions);

        printf("Allocating state %d\n", to_state->id);
        
        // linking new state to the "so far" end of the pattern
        pm_goto_set(from_state, pattern[i], to_state);

        // forwarding to the next step
        from_state = to_state;
        to_state = NULL;
    }   

    // adding the pattern to the final state - accepting state
    dbllist_node_t *output_node = dbllist_head(from_state->output);
    while(output_node != NULL) {
        if(strcmp(dbllist_data(output_node), pattern) == 0) {
            printf("pattern already exist in node");
            return -1;
        }
        output_node = dbllist_next(output_node);
    }
    dbllist_append(from_state->output, pattern);
    return 0;
}

// step forward in the tree, gets state and label - returns state, if no edge exist return NULL
pm_state_t* pm_goto_get(pm_state_t *state,unsigned char symbol) {
    dbllist_node_t *node = dbllist_head(state->_transitions);
    if(node == NULL) { return NULL; }

    // traversing over the transition list looking for the next state with the @param symbol
    pm_labeled_edge_t *edge;
    while(node != NULL) {
        edge = (pm_labeled_edge_t*)dbllist_data(node);
        if(((pm_labeled_edge_t*)dbllist_data(node))->label == symbol) {
            return edge->state;
        }
        node = dbllist_next(node);
    }
    return NULL;
}

// creating new edge in the tree linking to_state to from_state
int pm_goto_set(pm_state_t *from_state, unsigned char symbol, pm_state_t *to_state) {
    // create new edge to the to_state
    pm_labeled_edge_t *edge = (pm_labeled_edge_t*)malloc(sizeof(pm_labeled_edge_t));
    if(edge == NULL) {
        perror("malloc failed");
        return -1;
    }
    edge->label = symbol;
    edge->state = to_state;

    // adding new edge to the from_state transition list
    dbllist_append(from_state->_transitions, edge);
    printf("%d -> %c -> %d\n", from_state->id, symbol, to_state->id);
    return 0;
}

// creating fail transition in the pm machine (red arrows)
int pm_makeFSM(pm_t *pm) {
    // queue of states
    dbllist_t *states_queue = (dbllist_t*)malloc(sizeof(dbllist_t));
    dbllist_init(states_queue);

    pm_state_t *root_state = pm->zerostate;
    if(root_state == NULL || root_state->_transitions == NULL) { return -1; }

    // inserting states in depth 1 to the queue
    dbllist_node_t *edge_node = dbllist_head(root_state->_transitions);
    while(edge_node != NULL) {
        pm_state_t *state = ((pm_labeled_edge_t*)(dbllist_data(edge_node)))->state;
        dbllist_append(states_queue, state);
        // failure transitions of states in depth 1 is the root
        state->fail = root_state;
        edge_node = dbllist_next(edge_node);
    }

    // for each node in the machine
    while(dbllist_size(states_queue) > 0) {
        // state_node = dequeue(states_queue)
        dbllist_node_t *state_node = dbllist_head(states_queue);
        pm_state_t *parent_state = dbllist_data(state_node);
        dbllist_remove(states_queue, state_node, DBLLIST_LEAVE_DATA);
        edge_node = dbllist_head(parent_state->_transitions);

        // going over all the edges of the node
        while(edge_node != NULL) {
            // add the son to the list - allow the BFS traversal
            pm_state_t *child_state = ((pm_labeled_edge_t*)(dbllist_data(edge_node)))->state;
            dbllist_append(states_queue, child_state);
            char symbol = ((pm_labeled_edge_t*)dbllist_data(edge_node))->label;
            pm_state_t *fail_state = parent_state->fail;
            pm_labeled_edge_t *temp_edge, *fail_edge = NULL;
            // tracing back fail states to find the longest suffix of the node
            while(fail_state != NULL) {
                // if we didnt find edge than fail transition will be to zerostate
                if(fail_state->id == 0) {
                    child_state->fail = root_state;
                }
                // if we find such edge - setting child_state->fail
                if(pm_goto_get(fail_state, symbol) != NULL) {
                    child_state->fail = pm_goto_get(fail_state, symbol);
                    break;
                }
                fail_state = fail_state->fail;
            }
            printf("setting f(%d) = %d\n", child_state->id, child_state->fail->id);
            
            // union of output lists
            dbllist_t *fail_output = child_state->fail->output;
            if(fail_output != NULL) {
                dbllist_node_t *fail_output_node = dbllist_head(fail_output);
                while(fail_output_node != NULL) {
                    dbllist_append(child_state->output, dbllist_data(fail_output_node));
                    fail_output_node = dbllist_next(fail_output_node);
                }
            }
            edge_node = dbllist_next(edge_node);
        }
    }
    dbllist_destroy(states_queue, DBLLIST_LEAVE_DATA);
    free(states_queue);
    states_queue = NULL;
    return 0;
}

/*
* traversing through text looking for the patterns from the pm machine in it - returns list of pm_match_t
* each pm_match represent a pattern found in the text
*/ 
dbllist_t* pm_fsm_search(pm_state_t *state,unsigned char *text, _size_t n) {
    if(state == NULL) {
        return NULL;
    }
    // list of pm_match
    dbllist_t* search_result = (dbllist_t*)malloc(sizeof(dbllist_t));
    if(search_result == NULL) {
        perror("malloc failed");
        return NULL;
    }
    dbllist_init(search_result);

    for (int i = 0; i < n; i++) {
        // if no forward edge exist step to the fail until we reach root state
        while(pm_goto_get(state, text[i]) == NULL && state->id != 0) {
            state = state->fail;
        }
        if(pm_goto_get(state, text[i]) != NULL) {
            state = pm_goto_get(state, text[i]);
        }
        // if we reached accepting state - create pm matches and add them to list
        if(state->output != NULL && state->output->size > 0) {
            dbllist_node_t* output_node = dbllist_head(state->output);
            while(output_node != NULL) {
                pm_match_t *match = (pm_match_t*)malloc(sizeof(pm_match_t));
                if(match == NULL) {
                    perror("malloc failed");
                    return NULL;
                }
                match->pattern = output_node->data;
                match->start_pos = i - strlen(match->pattern) + 1;
                match->end_pos = i;
                match->fstate = state;
                dbllist_append(search_result, match);
                output_node = dbllist_next(output_node);
            }
        }
    }
    return search_result;
}

// freeing the pattern matching maching (tree) using BFS
void pm_destroy(pm_t *pm) {
    if(pm == NULL) { 
        return; 
    }
    if(pm->newstate < 1) {
        free(pm);
        return;
    }
    // list of states to help free them
    dbllist_t *states_queue = (dbllist_t*)malloc(sizeof(dbllist_t));
    dbllist_init(states_queue);
    dbllist_node_t *edge_node;
    
    pm_state_t *state = pm->zerostate;
    dbllist_append(states_queue, state);
    
    while(dbllist_size(states_queue) > 0) {
        dbllist_node_t *state_node = dbllist_head(states_queue);
        state = dbllist_data(state_node);
        edge_node = dbllist_head(state->_transitions);
        
        // enqueue
        while(edge_node != NULL) {
            pm_state_t *child_state = ((pm_labeled_edge_t*)(dbllist_data(edge_node)))->state;
            dbllist_append(states_queue, child_state);
            edge_node = dbllist_next(edge_node);
        }
        dbllist_destroy(state->output, DBLLIST_LEAVE_DATA);
        free(state->output);
        state->output = NULL;
        dbllist_destroy(state->_transitions, DBLLIST_FREE_DATA);
        free(state->_transitions);
        state->_transitions = NULL;
        dbllist_remove(states_queue, state_node, DBLLIST_FREE_DATA);
    }
    dbllist_destroy(states_queue, DBLLIST_FREE_DATA);
    free(states_queue);
    states_queue = NULL;
}