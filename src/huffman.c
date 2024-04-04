#include "huffman.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct Node {
    struct Node *left;
    struct Node *right;
    unsigned int weight;
    char label;
};

static void printNode( const struct Node *node ) {
    printf( "Node Address:     %p\n", node );
    printf( "     Label:       %c\n", node->label );
    printf( "     Weight:      %u\n", node->weight );
    printf( "     Left Child:  %p\n", node->left );
    printf( "     Right Child: %p\n", node->right );
}

static struct Node combineAndResetNodes( struct Node *node1, struct Node *node2 ) {
    struct Node newNode = ( struct Node ) { node1, node2, node1->weight + node2->weight, NULL };
    node1->weight = UINT_MAX;
    node2->weight = UINT_MAX;
    return newNode; 
}

static int orderNodesDescending( const void *node1, const void *node2 ) {
    struct Node *n1 = ( struct Node * ) node1;
    struct Node *n2 = ( struct Node * ) node2;
    return n1->weight - n2->weight; 
}

void huffman( const char *stringToCompress, const size_t stringLength ) {
    unsigned int characterCounts[256] = {0};
    unsigned int numUniqueCharacters = 0;

    for ( size_t i = 0; i < stringLength; ++i ) {
        //increment characterCounts while also checking if that was the first
        //time that character was found
        numUniqueCharacters += ++characterCounts[stringToCompress[i]] == 1;
    }

    if ( !numUniqueCharacters ) return;

    char output[512] = {0}; //double the range of byte to account for commas
                            //null byte will always be able to be added to end because the 
                            //final comma is replaced by it
    char *outputPointer = &output[0];
    struct Node queue1[numUniqueCharacters];
    struct Node queue2[numUniqueCharacters];
    uint8_t queue1Index = 0; 
    uint8_t queue2Index = 0; 

    printf( "Number of unique characters: %u\n", numUniqueCharacters ); 
    //iterate through all possible chars, not limiting to 127 now because I
    //want this to be a generic algorithm, not just focused on alphabets.
    //for now, I am just testing with strings, so am avoiding 0 because it causes
    //output to print nothing
    for ( uint8_t i = 1; i != 0; ++i ) { //goes from 1 to 255
        if ( characterCounts[i] ) {
            printf( "%uc: %i\n", i, characterCounts[i] );
            queue1[queue1Index] = ( struct Node ) { NULL, NULL, characterCounts[i], i };
            queue2[queue1Index++] = ( struct Node ) { NULL, NULL, UINT_MAX, 0 };
        } else {
            *outputPointer++ = i;
            *outputPointer++ = ',';
        }
    }
    //remove final comma, replace with null byte
    --outputPointer; 
    *outputPointer = '\0';
    printf( "No counts: %s\n", output );

    qsort( queue1, numUniqueCharacters, sizeof( struct Node ), orderNodesDescending ); 

    //make the final tree
    //there will always be n - 1 steps to make it, so start by decrementing
    //the value
    while ( --numUniqueCharacters ) {
        bool queue1LT = queue1[0].weight < queue2[0].weight; 
        bool queue2LT;
        //queue2Index -= queue1LT == false; //decrement the index if queue2 element is taken
        struct Node *p_node1 = queue1LT ? &queue1[0] : &queue2[0];
        struct Node *p_node2;
        if ( queue1LT ) {
            queue2LT = queue2[0].weight < queue1[1].weight; 
            p_node2 = queue2LT ? &queue2[0] : &queue1[1];
            //queue2Index -= queue2LT; 
        } else {
            queue2LT = queue2[1].weight < queue1[0].weight;
            p_node2 = queue2LT ?  &queue2[1] : &queue1[0];
            //queue2Index -= queue2LT;
        }
        //need to move all of the elements forward
        queue2[queue2Index++] = combineAndResetNodes( p_node1, p_node2 );

        qsort( queue1, numUniqueCharacters, sizeof( struct Node ), orderNodesDescending );
        qsort( queue2, numUniqueCharacters, sizeof( struct Node ), orderNodesDescending );
    }
    printNode( &queue2[0] );
}
