#ifndef DELTA_H
#define DELTA_H
#include <stdint.h>
#include <stdlib.h>

typedef struct String {
    char *content;
    size_t size;
} String;

String deltaFileIntoString( const char *inputFileName );
void deltaFileIntoFile( const char *inputFileName, const char *outputFileName );

#endif
