#include <stdio.h>
#include <string.h>
#include "horspool.h"
#include "scanner.h"

int scanFile(char filename[],
             char patterns[][50],
             int  patternCount,
             int  flaggedLines[],
             int *flaggedCount)
{
    FILE *fp     = fopen(filename, "r");
    FILE *report = fopen("report.txt", "a");

    if (fp == NULL)
    {
        printf("    Cannot open file: %s\n", filename);
        if (report) fclose(report);
        return 0;
    }

    fprintf(report, "\n========================================\n");
    fprintf(report, "FILE: %s\n", filename);
    fprintf(report, "========================================\n\n");

    char line[512];
    int  lineNo     = 1;
    int  violations = 0;

    while (fgets(line, sizeof(line), fp))
    {
        /* Strip trailing newline for cleaner report output */
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        for (int i = 0; i < patternCount; i++)
        {
            if (horspoolSearch(line, patterns[i]) != -1)
            {
                violations++;

                printf("    [!] %-10s  ->  Line %d: %s\n",
                       patterns[i], lineNo, line);

                fprintf(report, "Violation Type : %s\n",   patterns[i]);
                fprintf(report, "Line Number    : %d\n",   lineNo);
                fprintf(report, "Matched Code   : %s\n",   line);
                fprintf(report, "----------------------------------------\n");

                /* Record flagged line (no duplicates) */
                if (flaggedLines && flaggedCount && *flaggedCount < 100)
                {
                    int dup = 0;
                    for (int k = 0; k < *flaggedCount; k++)
                        if (flaggedLines[k] == lineNo) { dup = 1; break; }
                    if (!dup)
                        flaggedLines[(*flaggedCount)++] = lineNo;
                }
            }
        }
        lineNo++;
    }

    if (violations == 0)
        fprintf(report, "  No violations found.\n");

    fclose(fp);
    fclose(report);
    return violations;
}