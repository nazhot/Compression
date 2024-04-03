#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>


void huffman( char *stringToCompress, unsigned int stringLength ) {
    unsigned int characterCounts[256] = {0};
    for ( size_t i = 0; i < stringLength; ++i ) {
        characterCounts[stringToCompress[i]]++;
    }

    char output[512] = {0};
    char *outputPointer = &output[0];
    
    for ( int i = 1; i < 256; ++i ) {
        if ( characterCounts[i] ) {
            printf( "%c: %i\n", i, characterCounts[i] );
        } else {
            *outputPointer++ = i;
            *outputPointer++ = ',';
        }
    }
    printf( "No counts: %s\n", output );
}
