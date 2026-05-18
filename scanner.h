#ifndef SCANNER_H
#define SCANNER_H

/* Scans filename for all patterns using Horspool.
   Appends results to report.txt.
   Stores flagged line numbers in flaggedLines[].
   Returns total violation count. */
int scanFile(char filename[],
             char patterns[][50],
             int  patternCount,
             int  flaggedLines[],
             int *flaggedCount);

#endif