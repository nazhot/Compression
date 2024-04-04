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

static int orderNodesDescending( const void *node1, const void *node2 ) {
    struct Node *n1 = ( struct Node * ) node1;
    struct Node *n2 = ( struct Node * ) node2;
    return n1->weight - n2->weight; 
}

void huffman( char *stringToCompress, size_t stringLength ) {
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
            queue1[queue1Index++] = ( struct Node ) { NULL, NULL, characterCounts[i], NULL };
        } else {
            *outputPointer++ = i;
            *outputPointer++ = ',';
        }
    }
    --outputPointer; 
    *outputPointer = '\0';
    printf( "No counts: %s\n", output );

    qsort( queue1, sizeof( struct Node ), numUniqueCharacters, orderNodesDescending ); 


}
