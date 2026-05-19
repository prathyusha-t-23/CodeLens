#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "horspool.h"
#include "naive.h"
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

    /* --- Time Comparison on test file --- */
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *buf = (char *)malloc(fsize + 1);
    if (buf)
    {
        size_t readBytes = fread(buf, 1, fsize, fp);
        buf[readBytes] = '\0';
        rewind(fp);

        int dummyMatches[2000];
        int iterations = 50000;
        if (fsize > 200000)      iterations = 100;
        else if (fsize > 50000)  iterations = 500;
        else if (fsize > 10000)  iterations = 2000;
        
        clock_t t0 = clock();
        for (int iter = 0; iter < iterations; iter++)
        {
            for (int i = 0; i < patternCount; i++)
                horspoolSearchAll(buf, patterns[i], dummyMatches, 2000);
        }
        clock_t t1 = clock();

        for (int iter = 0; iter < iterations; iter++)
        {
            for (int i = 0; i < patternCount; i++)
                naiveSearchAll(buf, patterns[i], dummyMatches, 2000);
        }
        clock_t t2 = clock();

        double hms = (double)(t1 - t0) / CLOCKS_PER_SEC * 1000.0;
        double nms = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000.0;

        fprintf(report, "Search Times Comparison:\n");
        fprintf(report, "  Boyer-Moore-Horspool: %.4f ms\n", hms);
        fprintf(report, "  Brute-Force Naive   : %.4f ms\n\n", nms);

        free(buf);
    }

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