#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

    char output[512] = {0};
    char *outputPointer = &output[0];
    struct Node queue1[numUniqueCharacters];
    struct Node queue2[numUniqueCharacters];
    uint8_t queue1Index = 0; 
    uint8_t queue2Index = 0; 

    printf( "Number of unique characters: %u\n", numUniqueCharacters ); 
    for ( uint8_t i = 1; i != 0; ++i ) { //goes from 1 to 255
        if ( characterCounts[i] ) {
            printf( "%uc: %i\n", i, characterCounts[i] );
            queue1[queue1Index++] = ( struct Node ) { NULL, NULL, characterCounts[i], i };
        } else {
            *outputPointer++ = i;
            *outputPointer++ = ',';
        }
    }
    --outputPointer; 
    *outputPointer = '\0';
    printf( "No counts: %s\n", output );

    qsort( queue1, numUniqueCharacters, sizeof( struct Node ), orderNodesDescending ); 
}
