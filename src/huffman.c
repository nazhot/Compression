#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>

struct Node {
    struct Node *left;
    struct Node *right;
    unsigned int weight;
};


void huffman( char *stringToCompress, unsigned int stringLength ) {
    unsigned int characterCounts[256] = {0};
    unsigned int numUniqueCharacters = 0;

    for ( size_t i = 0; i < stringLength; ++i ) {
        //increment characterCounts while also checking if that was the first
        //time that character was found
        numUniqueCharacters += ++characterCounts[stringToCompress[i]] == 1;
    }

    char output[512] = {0};
    char *outputPointer = &output[0];

    printf( "Number of unique characters: %u\n", numUniqueCharacters ); 
    for ( int i = 1; i < 256; ++i ) {
        if ( characterCounts[i] ) {
            printf( "%c: %i\n", i, characterCounts[i] );
        } else {
            *outputPointer++ = i;
            *outputPointer++ = ',';
        }
    }
    --outputPointer; 
    *outputPointer = '\0';
    printf( "No counts: %s\n", output );
}
