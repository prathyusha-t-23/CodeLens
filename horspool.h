#ifndef HORSPOOL_H
#define HORSPOOL_H

#define ALPHABET_SIZE 256

void buildShiftTable(const char *pattern, int m, int table[ALPHABET_SIZE]);
void printShiftTable(const char *pattern);

int  horspoolSearch(const char *text, const char *pattern);
int  horspoolSearchAll(const char *text, const char *pattern,
                       int matches[], int maxMatches);

#endif
