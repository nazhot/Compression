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

struct PriorityNode {
    struct Node *address;
    unsigned int weight;
};

struct EncoderEntry {
    char character;
    uint64_t key;
    uint8_t keyLength;
};

static void printNode( const struct Node *node ) {
    printf( "Node Address:     %p\n", node );
    printf( "     Label:       %c\n", node->label );
    printf( "     Weight:      %u\n", node->weight );
    printf( "     Left Child:  %p\n", node->left );
    printf( "     Right Child: %p\n", node->right );
}

static void printTree( struct Node *node, char *string, unsigned int index ) {
    if ( node->left ) {
        string[index] = '0';
        printTree( node->left, string, index + 1 );
        string[index] = '\0';
    }
    if ( node->right ) {
        string[index] = '1';
        printTree( node->right, string, index + 1 );
        string[index] = '\0';
    }

    if ( node->label ) {
        printf( "%c: %s\n", node->label, string );
    }
}

static void setupEncoderTable( struct Node *node, struct EncoderEntry *table, uint64_t binary, unsigned int index, uint8_t *encoderTableIndex ) {
    if ( !node ) return;
    
    if ( node->label ) {
        table[*encoderTableIndex].character = node->label;
        table[*encoderTableIndex].key = binary;
        table[*encoderTableIndex].keyLength = index;
        *encoderTableIndex = *encoderTableIndex + 1;
        return;
    }

    setupEncoderTable( node->left, table, binary, index + 1, encoderTableIndex );
    setupEncoderTable( node->right, table, binary | ( 1 << index ), index + 1, encoderTableIndex );
}

static struct Node combineAndResetNodes( struct PriorityNode *node1, struct PriorityNode *node2 ) {
    struct Node newNode = ( struct Node ) { node1->address, node2->address, node1->weight + node2->weight, NULL };
    node1->weight = UINT_MAX;
    node2->weight = UINT_MAX;
    return newNode; 
}

static int orderPriorityNodesAscending( const void *node1, const void *node2 ) {
    const struct PriorityNode *n1 = ( struct PriorityNode * ) node1;
    const struct PriorityNode *n2 = ( struct PriorityNode * ) node2;
    return n1->weight <=  n2->weight ? -1 : 1;
}

static void shiftPriorityNodeArrayDown( struct PriorityNode * const array, const unsigned int arrayLength, const unsigned int numShiftDown ) {
    if ( arrayLength <= numShiftDown ) return;

    if ( numShiftDown == 2 ) array[1] = ( struct PriorityNode ) { NULL, UINT_MAX }; //deals with a situation where if it's a length of 3, and a shiftdown of 2, array[1] wouldn't be touched and would mess up future calcs
    unsigned int index = numShiftDown;
    while ( arrayLength - index ) {
        array[index - numShiftDown] = array[index]; 
        ++index;
    }
    for ( unsigned int i = 0; i < numShiftDown; i++ ) {
        array[arrayLength - 1 - i] = ( struct PriorityNode ) { NULL, UINT_MAX };
    }
}

static int orderEncoderEntryAscendingByCharacter( const void *entry1, const void *entry2 ) {
    const struct EncoderEntry *e1 = ( struct EncoderEntry * ) entry1;
    const struct EncoderEntry *e2 = ( struct EncoderEntry * ) entry2;
    return e1->character < e2->character ? -1 : 1;
}
static int orderEncoderEntryAscendingByKeyLength( const void *entry1, const void *entry2 ) {
    const struct EncoderEntry *e1 = ( struct EncoderEntry * ) entry1;
    const struct EncoderEntry *e2 = ( struct EncoderEntry * ) entry2;
    return e1->keyLength <= e2->keyLength ? -1 : 1;
}

static void updateTableToCanonical( struct EncoderEntry *table ) {

}

void huffman_encode( const char *stringToCompress, const size_t stringLength ) {
    unsigned int characterCounts[256] = {0};
    unsigned int numUniqueCharacters = 0;

    for ( size_t i = 0; i < stringLength; ++i ) {
        //increment characterCounts while also checking if that was the first
        //time that character was found
        numUniqueCharacters += ++characterCounts[stringToCompress[i]] == 1;
    }

    if ( !numUniqueCharacters ) return;

    struct Node allNodes[numUniqueCharacters * 2 - 1];
    struct PriorityNode queue1[numUniqueCharacters];
    struct PriorityNode queue2[numUniqueCharacters];
    uint16_t allNodesIndex = 0;
    uint8_t queue1Index = 0; 
    uint8_t queue2Index = 0; 

    printf( "Number of unique characters: %u\n", numUniqueCharacters ); 
    //iterate through all possible chars, not limiting to 127 now because I
    //want this to be a generic algorithm, not just focused on alphabets.
    //for now, I am just testing with strings, so am avoiding 0 because it causes
    //output to print nothing
    for ( uint8_t i = 1; i != 0; ++i ) { //goes from 1 to 255
        if ( !characterCounts[i] ) continue;

        printf( "%c: %i\n", i, characterCounts[i] );
        allNodes[allNodesIndex] = ( struct Node ) { NULL, NULL, characterCounts[i], i };
        queue1[queue1Index] = ( struct PriorityNode ) { &allNodes[allNodesIndex++], characterCounts[i] };
        queue2[queue1Index++] = ( struct PriorityNode ) { NULL, UINT_MAX };
    }

    qsort( queue1, numUniqueCharacters, sizeof( struct PriorityNode ), orderPriorityNodesAscending ); 

    //make the final tree
    uint8_t numStepsNeeded = numUniqueCharacters - 1;
    while ( numStepsNeeded-- ) {
        const bool queue1LT = queue1[0].weight < queue2[0].weight; 
        bool queue2LT;
        struct PriorityNode *p_node1 = queue1LT ? &queue1[0] : &queue2[0];
        struct PriorityNode *p_node2;
        if ( queue1LT ) {
            queue2LT = queue2[0].weight < queue1[1].weight; 
            p_node2 = queue2LT ? &queue2[0] : &queue1[1];
        } else {
            queue2LT = queue2[1].weight < queue1[0].weight;
            p_node2 = queue2LT ?  &queue2[1] : &queue1[0];
        }
        queue2Index -= ( queue1LT == false ) + ( queue2LT );
        const unsigned int numTakenFromQueue1 = queue1LT + ( queue2LT == false ); //how many elements from q1 were taken
        const unsigned int numTakenFromQueue2 = 2 - numTakenFromQueue1; //how many elements from q2 were taken

        //create the new tree with the two lowest elements as its children
        allNodes[allNodesIndex] = combineAndResetNodes( p_node1, p_node2 );
        //shift both queues down by how many elements were taken from each
        shiftPriorityNodeArrayDown( queue1, numUniqueCharacters, numTakenFromQueue1 );
        shiftPriorityNodeArrayDown( queue2, numUniqueCharacters, numTakenFromQueue2 );
        //after shifting q2, place the new element at the end of the queue
        queue2[queue2Index++] = ( struct PriorityNode ) { &allNodes[allNodesIndex], allNodes[allNodesIndex].weight };
        ++allNodesIndex;
    }

    char *temp = malloc( sizeof( char ) * 256 );
    printTree( &allNodes[allNodesIndex - 1], temp, 0 );
    free( temp );
    struct EncoderEntry encoderTable[numUniqueCharacters];
    uint8_t encoderTransform[256] = {0};
    for ( int i = 0; i < numUniqueCharacters; ++i ) {
        encoderTable[i] = ( struct EncoderEntry ) { 
                                                    .character = i,
                                                    .key = 0,
                                                    .keyLength = 255
                                                  };
    }
    uint8_t encoderTableIndex = 0;
    setupEncoderTable( &allNodes[allNodesIndex - 1], encoderTable, 0, 0, &encoderTableIndex );
    qsort( encoderTable, numUniqueCharacters, sizeof( struct EncoderEntry ), orderEncoderEntryAscendingByCharacter );
    qsort( encoderTable, numUniqueCharacters, sizeof( struct EncoderEntry ), orderEncoderEntryAscendingByKeyLength );
    for ( unsigned int i = 0; i < numUniqueCharacters; ++i ) {
        encoderTransform[encoderTable[i].character] = i;
    }

    for ( unsigned int i = 0; i < numUniqueCharacters; ++i ) {
        if ( i == 0 ) {
            encoderTable[0].key = 0; //will expand out to all zeroes, keyLength tells the encoder how many 0 bits to input to file
            continue;
        }
        unsigned int newKey = encoderTable[i - 1].key + 1;
        unsigned int length = 0;
        for ( unsigned int j = 0; j < sizeof( unsigned int ) * 8; ++j ) {
            if ( newKey >> j & 1 ) {
                length = j + 1;
            }
        }
        while ( length++ < encoderTable[i].keyLength ) {
            newKey = newKey << 1;
        }
        encoderTable[i].key = newKey;
    }

    char outputText[stringLength];
    unsigned int outputTextIndex = 0;
    uint8_t bitOffset = 0;
    char currentByte = '\0';

    for ( int i = 0; i < stringLength; ++i ) {
        char character = stringToCompress[i];
        struct EncoderEntry entry = encoderTable[encoderTransform[character]];
        //printf( "%c: %u (%u)\n", character, entry.key, entry.keyLength );
        for ( int j = 0; j < entry.keyLength; ++j ) {
            if ( bitOffset > 7 ) { //end of buffer
                outputText[outputTextIndex++] = currentByte;
                currentByte = '\0';
                bitOffset = 0;
            }
            currentByte |= ( ( entry.key >> j ) & 1 ) << bitOffset; //key bit shifted j times to the right (0 or 1), and shift that to the left bitOffset times
            bitOffset++;
        }
    }
    outputText[outputTextIndex++] = currentByte;
    FILE *outputFile = fopen( "output", "wb" );
    fwrite( outputText, 1, outputTextIndex, outputFile );
    fclose( outputFile );
/*

    FILE *inputFile = fopen( "output", "rb" );
    char encodedBinary = '\0';
    struct Node *currentNode = &allNodes[allNodesIndex - 1];
    while ( fread( &encodedBinary, 1, 1, inputFile ) ) {
        for ( int i = 0; i < 8; ++i ) {
            if ( currentNode->label ) {
                printf( "%c", currentNode->label );
                currentNode = &allNodes[allNodesIndex - 1];
            }
            if ( encodedBinary >> i & 1 ) {
                currentNode = currentNode->right;
            } else {
                currentNode = currentNode->left;
            }
        }
    }
    printf( "\n" );
*/
}

void huffman_decode( const char *stringToDecompress ) {

}
