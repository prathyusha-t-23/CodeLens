#include <stdio.h>
#include <string.h>
#include "file_access.h"

#define MAX_LINES 20000

/* ------------------------------------------------------------------
   Sequential Access
   Reads the file from top to bottom, line by line.
   Simulates OS sequential file access.
   ------------------------------------------------------------------ */
void sequentialAccess(char filename[])
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("    [sequential] Error opening file: %s\n", filename);
        return;
    }

    printf("    --- Sequential Access: %s ---\n", filename);

    char line[512];
    int  lineNo = 1;

    while (fgets(line, sizeof(line), file))
    {
        /* Strip newline */
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        printf("    %4d | %s\n", lineNo++, line);
    }

    fclose(file);
    printf("    --- End of sequential read ---\n\n");
}

/* ------------------------------------------------------------------
   Build byte-offset index
   Stores the file position (ftell) at the START of each line.
   Returns total number of lines indexed.
   ------------------------------------------------------------------ */
static int buildLineIndex(FILE *file, long offsets[], int maxLines)
{
    int count = 0;
    char line[512];

    rewind(file);

    while (count < maxLines)
    {
        long pos = ftell(file);
        if (fgets(line, sizeof(line), file) == NULL) break;
        offsets[count++] = pos;
    }

    return count;
}

/* ------------------------------------------------------------------
   Direct Access — single line
   Uses fseek() on a pre-built byte-offset table to jump
   directly to the requested line without scanning top-to-bottom.
   ------------------------------------------------------------------ */
void directAccess(char filename[], int targetLine)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("    [direct] Error opening file: %s\n", filename);
        return;
    }

    static long offsets[MAX_LINES];
    int totalLines = buildLineIndex(file, offsets, MAX_LINES);

    printf("\n    ===== Direct Access =====\n");

    if (targetLine < 1 || targetLine > totalLines)
    {
        printf("    Line %d out of range (file has %d lines).\n",
               targetLine, totalLines);
        fclose(file);
        return;
    }

    /* Jump directly via byte offset — O(1) seek */
    fseek(file, offsets[targetLine - 1], SEEK_SET);

    char line[512];
    fgets(line, sizeof(line), file);

    int len = (int)strlen(line);
    while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
        line[--len] = '\0';

    printf("    Jumped to Line %d: %s\n", targetLine, line);
    printf("    =========================\n\n");

    fclose(file);
}

/* ------------------------------------------------------------------
   Direct Access Demo — multiple flagged lines
   Builds the offset index once, then seeks to each flagged line.
   ------------------------------------------------------------------ */
void directAccessDemo(char filename[], int lines[], int count)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("    [direct demo] Error opening file: %s\n", filename);
        return;
    }

    static long offsets[MAX_LINES];
    int totalLines = buildLineIndex(file, offsets, MAX_LINES);

    printf("\n    ===== Direct Access Demo: %s =====\n", filename);
    printf("    Byte-offset index built for %d lines.\n\n", totalLines);

    char line[512];

    for (int i = 0; i < count; i++)
    {
        int target = lines[i];
        if (target < 1 || target > totalLines) continue;

        fseek(file, offsets[target - 1], SEEK_SET);
        fgets(line, sizeof(line), file);

        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        printf("    [Line %4d | offset %6ld] %s\n",
               target, offsets[target - 1], line);
    }

    printf("    ==========================================\n\n");
    fclose(file);
}