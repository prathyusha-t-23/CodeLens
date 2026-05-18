#include <stdio.h>
#include <string.h>

#include "horspool.h"

void scanFile(char filename[], char patterns[][50], int patternCount)
{
    FILE *fp;
    FILE *report;

    char line[256];

    int lineNo = 1;

    fp = fopen(filename, "r");

    if(fp == NULL)
    {
        printf("Cannot open file\n");
        return;
    }

    report = fopen("report.txt", "a");

    fprintf(report,
            "\n========================================\n");

    fprintf(report,
            "FILE: %s\n",
            filename);

    fprintf(report,
            "========================================\n\n");

    while(fgets(line, sizeof(line), fp))
    {
        for(int i = 0; i < patternCount; i++)
        {
            int result =
                horspoolSearch(line, patterns[i]);

            if(result != -1)
            {
                printf("Violation Found in Line %d\n",
                       lineNo);

                fprintf(report,
                        "Violation Type : %s\n",
                        patterns[i]);

                fprintf(report,
                        "Line Number    : %d\n",
                        lineNo);

                fprintf(report,
                        "Matched Code   : %s\n",
                        line);

                fprintf(report,
                        "----------------------------------------\n");
            }
        }

        lineNo++;
    }

    fclose(fp);
    fclose(report);
}