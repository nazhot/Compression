#include "huffman.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

//makes up the encoding/decoding tree
typedef struct Node {
    struct Node *left;  //represented by '0' in key
    struct Node *right; //represented by '1' in key
    uint weight;        //number of times character appears in file
    char character;      
} Node;

//used to save space when creating the encoding tree
typedef struct PriorityNode {
    struct Node *address; 
    uint weight;  
} PriorityNode;

//holds information on encoding/decoding characters
typedef struct EncoderEntry {
    char character;    
    uint64_t key;      //binary key used to encode/decode that character
                       //keys that use more than 64 bits will overflow and not be stored
                       //TODO: update to format that can handle up to the possible 255 length
    uint8_t keyLength; //how many bits make up the key, 1 <= keyLength <= 255
} EncoderEntry;

/*
* Recursively print the whole tree structure, giving the key of each character
* within it
*/
static void printTree( const Node *node ) {

    static char key[256] = {0}; //stores the path to each node, always ends back at all '\0'
    static uint treeLayer = 0;  //how deep down into the tree the function is, always returns to 0

    if ( !node ) { 
        return;
    } else if ( node->character ) {
        printf( "%c: %s\n", node->character, key );
        return;
    }

    key[treeLayer++] = '0';
    printTree( node->left );
    key[treeLayer - 1] = '1';
    printTree( node->right );
    key[--treeLayer] = '\0';

}

/*
* Prints the bits of the binary given up to the requested length, prints in
* the same order as it appears left to right when you use left/right shift
*/
static void printBinary( const uint binary, const uint length ) {
    for ( uint i = 0; i < length; ++i ) {
       printf( "%c", binary >> ( length - 1 - i ) & 1 ? '1' : '0' );
    }
    printf( " (%u)", binary );
}

/*
* Runs the initial setup of the encoder table, which holds information about the
* key and keyLength for each character.
*
* Can NOT be used multiple times in one run, tableIndex will not be
* set back to 0
*/
static void setupEncoderTable( const Node *node, EncoderEntry* const table ) {

    static uint64_t key = 0;
    static uint treeLayer = 0;
    static uint8_t tableIndex = 0;

    if ( !node ) {
        return;
    } else if ( node->character ) {
        table[tableIndex] = ( EncoderEntry ) {
                                               .character = node->character,
                                               .key = key,
                                               .keyLength = treeLayer
                                             };
        tableIndex++;
        return;
    }
    treeLayer++;
    setupEncoderTable( node->left, table );
    key ^=  1 << ( treeLayer - 1 );
    setupEncoderTable( node->right, table );
    key ^= 1 << ( treeLayer - 1 ); 
    treeLayer--;
}

/*
* This assumes that the weights will not overflow, no check is done. It also
* assumes that node1 and node2 are both non-NULL, passing a NULL value in
* either would crash the program. It also directly modifies the values
* within node1 and node2.
*/
static Node combineAndResetNodes( PriorityNode* const node1, PriorityNode* const node2 ) {
    Node newNode = ( Node ) { 
                              .left = node1->address,
                              .right = node2->address,
                              .weight = node1->weight + node2->weight,
                              .character = '\0'
                            };
    node1->weight = UINT_MAX;
    node2->weight = UINT_MAX;
    return newNode; 
}

static int orderPriorityNodesAscending( const void *node1, const void *node2 ) {
    const PriorityNode *n1 = ( PriorityNode * ) node1;
    const PriorityNode *n2 = ( PriorityNode * ) node2;
    return n1->weight <=  n2->weight ? -1 : 1;
}

static void shiftPriorityNodeArrayDown( PriorityNode* const array, const uint arrayLength, const uint numShiftDown ) {
    if ( arrayLength <= numShiftDown ) return;

    if ( numShiftDown == 2 ) array[1] = ( PriorityNode ) { 
                                                           .address =  NULL,
                                                           .weight = UINT_MAX
                                                         }; //deals with a situation where if it's a length of 3, and a shiftdown of 2, array[1] wouldn't be touched and would mess up future calcs
    //shift the elements down the array
    uint index = numShiftDown;
    while ( arrayLength - index ) {
        array[index - numShiftDown] = array[index]; 
        ++index;
    }
    
    //make the final moved elements have maxed out weight with no address to avoid
    //moving them multiple times in future calls
    for ( uint i = 0; i < numShiftDown; ++i ) {
        array[arrayLength - 1 - i] = ( PriorityNode ) {
                                                        .address = NULL,
                                                        .weight = UINT_MAX
                                                      };
    }
}

//ascending is referring to the literal ascii code for characters, thinking of
//them as characters it could be said they're descending (A, B, C, ...)
static int orderEncoderEntryAscendingByCharacter( const void *entry1, const void *entry2 ) {
    const EncoderEntry *e1 = ( EncoderEntry * ) entry1;
    const EncoderEntry *e2 = ( EncoderEntry * ) entry2;
    return e1->character < e2->character ? -1 : 1;
}
static int orderEncoderEntryAscendingByKeyLength( const void *entry1, const void *entry2 ) {
    const EncoderEntry *e1 = ( EncoderEntry * ) entry1;
    const EncoderEntry *e2 = ( EncoderEntry * ) entry2;
    return e1->keyLength <= e2->keyLength ? -1 : 1;
}

void huffman_encode( const char *inputFileName, const char *outputFileName ) {
    FILE *inputFile = fopen( inputFileName, "r" );
    if ( !inputFile ) {
        fprintf( stderr, "Could not open input file %s (huffman encode)\n", inputFileName );
        fprintf( stderr, "Error number: %i\n", errno );
        return;
    }

    uint characterCounts[256] = {0};
    uint numUniqueCharacters = 0;
    char currentCharacter = '\0'; //used to store the character from the input file
    uint inputFileLength = 0;

    //read the file one character at a time
    while ( fread( &currentCharacter, 1, 1, inputFile ) ) {
        numUniqueCharacters += ++characterCounts[currentCharacter] == 1;
        inputFileLength++;
    }

    if ( !numUniqueCharacters ) { //empty file
        fprintf( stderr, "%s has not contents, exiting\n", inputFileName );
        return;
    }

    Node allNodes[numUniqueCharacters * 2 - 1]; //total nodes in the tree, is
                                                //n (total number of characters)
                                                //+ (n - 1) (number of combined
                                                //nodes
    PriorityNode queue1[numUniqueCharacters]; //initially holds all of the character leaves, only shrinks
    PriorityNode queue2[numUniqueCharacters]; //initially empty, fills up with combination Nodes. Can grow and shrink, always gains the newest Node created

    uint16_t allNodesIndex = 0; //has to be able to go up to 256 * 2 - 1 = 511, so uint16
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
        allNodes[allNodesIndex] = ( Node ) { NULL, NULL, characterCounts[i], i };
        queue1[queue1Index] = ( PriorityNode ) { &allNodes[allNodesIndex++], characterCounts[i] };
        queue2[queue1Index++] = ( PriorityNode ) { NULL, UINT_MAX };
    }

    qsort( queue1, numUniqueCharacters, sizeof( PriorityNode ), orderPriorityNodesAscending ); 

    //make the final tree
    uint8_t numStepsNeeded = numUniqueCharacters - 1;
    while ( numStepsNeeded-- ) {
        const bool queue1LT = queue1[0].weight < queue2[0].weight; 
        bool queue2LT;
        PriorityNode *p_node1 = queue1LT ? &queue1[0] : &queue2[0];
        PriorityNode *p_node2;
        if ( queue1LT ) {
            queue2LT = queue2[0].weight < queue1[1].weight; 
            p_node2 = queue2LT ? &queue2[0] : &queue1[1];
        } else {
            queue2LT = queue2[1].weight < queue1[0].weight;
            p_node2 = queue2LT ?  &queue2[1] : &queue1[0];
        }
        queue2Index -= ( queue1LT == false ) + ( queue2LT );
        const uint numTakenFromQueue1 = queue1LT + ( queue2LT == false ); //how many elements from q1 were taken
        const uint numTakenFromQueue2 = 2 - numTakenFromQueue1; //how many elements from q2 were taken

        //create the new tree with the two lowest elements as its children
        allNodes[allNodesIndex] = combineAndResetNodes( p_node1, p_node2 );
        //shift both queues down by how many elements were taken from each
        shiftPriorityNodeArrayDown( queue1, numUniqueCharacters, numTakenFromQueue1 );
        shiftPriorityNodeArrayDown( queue2, numUniqueCharacters, numTakenFromQueue2 );
        //after shifting q2, place the new element at the end of the queue
        queue2[queue2Index++] = ( PriorityNode ) { &allNodes[allNodesIndex], allNodes[allNodesIndex].weight };
        ++allNodesIndex;
    }

    printTree( &allNodes[allNodesIndex - 1] );
    EncoderEntry encoderTable[numUniqueCharacters];
    uint8_t encoderTransform[256] = {0};
    uint8_t numKeyLength[256] = {0}; //track how many of each key length there are for encoding the tree
    for ( int i = 0; i < numUniqueCharacters; ++i ) {
        encoderTable[i] = ( EncoderEntry ) { 
                                                    .character = i,
                                                    .key = 0,
                                                    .keyLength = 255
                                                  };
    }
    setupEncoderTable( &allNodes[allNodesIndex - 1], encoderTable );
    qsort( encoderTable, numUniqueCharacters, sizeof( EncoderEntry ), orderEncoderEntryAscendingByCharacter );
    qsort( encoderTable, numUniqueCharacters, sizeof( EncoderEntry ), orderEncoderEntryAscendingByKeyLength );

    for ( uint i = 0; i < numUniqueCharacters; ++i ) {
        encoderTransform[encoderTable[i].character] = i;
        numKeyLength[encoderTable[i].keyLength]++;
    }

    printf( "Encoder Table (encode)\n" );
    encoderTable[0].key = 0;
    for ( uint i = 1; i < numUniqueCharacters; ++i ) {
        encoderTable[i].key = ( encoderTable[i - 1].key + 1 ) << ( encoderTable[i].keyLength - encoderTable[i - 1].keyLength );
    }

    for ( int i = 0; i < numUniqueCharacters; ++i ) {
        printf( "%c: ", encoderTable[i].character );
        printBinary( encoderTable[i].key, encoderTable[i].keyLength );
        printf( "\n" );
    }

    char outputText[inputFileLength];
    uint outputTextIndex = 0;
    uint8_t bitOffset = 0;
    char currentByte = '\0';
    if ( fseek( inputFile, 0, SEEK_SET ) ) { //reset file to beginning
        fprintf( stderr, "Could not reset file to beginning!" );
        return;
    }

    while ( fread( &currentCharacter, 1, 1, inputFile ) ) {
        EncoderEntry entry = encoderTable[encoderTransform[currentCharacter]];
        //printf( "%c: %u (%u)\n", character, entry.key, entry.keyLength );
        for ( int j = 0; j < entry.keyLength; ++j ) {
            if ( bitOffset > 7 ) { //end of buffer
                outputText[outputTextIndex++] = currentByte;
                currentByte = '\0';
                bitOffset = 0;
            }
            currentByte |= ( ( entry.key >> ( entry.keyLength - j - 1 ) ) & 1 ) << bitOffset; //key bit shifted j times to the right (0 or 1), and shift that to the left bitOffset times
            bitOffset++;
        }
    }
    outputText[outputTextIndex++] = currentByte;
    FILE *outputFile = fopen( outputFileName, "wb" );

    uint8_t maxKeyLength = encoderTable[numUniqueCharacters - 1].keyLength;
    fwrite( &maxKeyLength, 1, 1, outputFile );
    for ( uint i = 1; i < maxKeyLength + 1; ++i ) {
        fwrite( &numKeyLength[i], 1, 1, outputFile ); //later update to only write the bits that are needed (1 for key length 1, 2 for 2, etc)
    }
    for ( uint i = 0; i < numUniqueCharacters; ++i ) {
        fwrite( &encoderTable[i].character, 1, 1, outputFile );
    }

    fwrite( outputText, 1, outputTextIndex, outputFile );
    fclose( outputFile );
}

void huffman_decode( const char *inputFileName ) {
    FILE *inputFile = fopen( inputFileName, "r" );
    if ( !inputFile ) return;
    
    uint8_t maxKeyLength;
    fread( &maxKeyLength, 1, 1, inputFile );
    printf( "Max key length: %u\n", maxKeyLength );
    uint8_t numKeyLengths[maxKeyLength]; //index 0 is length 1
    uint8_t numUniqueCharacters = 0;
    for ( uint i = 0; i < maxKeyLength; ++i ) {
        fread( &numKeyLengths[i], 1, 1, inputFile );
        numUniqueCharacters += numKeyLengths[i];
        printf( "Keys @ length %u: %u\n", i + 1, numKeyLengths[i] );
    }
    printf( "Num unique characters: %u\n", numUniqueCharacters );
    EncoderEntry encoderTable[numUniqueCharacters];
    uint8_t currentKeyLength = 1;
    while ( !numKeyLengths[currentKeyLength - 1] ) currentKeyLength++;
    uint8_t runningTotal = numKeyLengths[currentKeyLength - 1];
    for ( uint i = 0; i < numUniqueCharacters; ++i ) {
        fread( &encoderTable[i].character, 1, 1, inputFile );
        while ( i >= runningTotal ) {
            runningTotal += numKeyLengths[++currentKeyLength - 1];
        }
        encoderTable[i].keyLength = currentKeyLength;
        if ( i == 0 ) {
            encoderTable[0].key = 0;
            continue;
        }

        encoderTable[i].key = ( encoderTable[i - 1].key + 1 ) << ( encoderTable[i].keyLength - encoderTable[i - 1].keyLength );
    }

    printf( "Encoder Table (decode)\n" );
    for ( int i = 0; i < numUniqueCharacters; ++i ) {
        printf( "%c: ", encoderTable[i].character );
        printBinary( encoderTable[i].key, encoderTable[i].keyLength );
        printf( "\n" );
    }

    Node allNodes[numUniqueCharacters * 2 - 1];
    uint allNodesIndex = 1;

    for ( uint i = 0; i < numUniqueCharacters * 2 - 1; ++i ) {
        allNodes[i] = ( Node ) { NULL, NULL, 0, '\0' };
    }
    
    //setup tree
    for ( uint i = 0; i < numUniqueCharacters; ++i ) {
        Node *currentNodeAddress = &allNodes[0];
        for ( uint j = 0; j < encoderTable[i].keyLength; ++j ) {
            bool nextIsRight = encoderTable[i].key >> ( encoderTable[i].keyLength - j - 1 ) & 1;
            Node *nextNodeAddress = nextIsRight ? currentNodeAddress->right :
                                           currentNodeAddress->left;
            if ( !nextNodeAddress ) {
                nextNodeAddress = &allNodes[allNodesIndex++]; //set it to the next available node (is all NULLs at this point
            }

            if ( nextIsRight ) {
                currentNodeAddress->right = nextNodeAddress;
            } else {
                currentNodeAddress->left = nextNodeAddress;
            }

            currentNodeAddress = nextNodeAddress;
        }
        currentNodeAddress->character = encoderTable[i].character;
    }

    char currentByte; 
    Node *currentNode = &allNodes[0];
    while ( fread( &currentByte, 1, 1, inputFile ) ) {
       for ( uint i = 0; i < 8; ++i ) {
            if ( !currentNode ) {
                break;
            }
            if ( currentNode->character ) {
                printf( "%c", currentNode->character );
                currentNode = &allNodes[0];
            }

            bool moveRight = currentByte >> i & 1;
            currentNode = moveRight ? currentNode->right : currentNode->left;
        }
    }
}
