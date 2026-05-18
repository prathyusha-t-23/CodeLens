#include <stdio.h>
#include <string.h>
#include "horspool.h"

/* Build the bad-character shift table for a given pattern */
void buildShiftTable(const char *pattern, int m, int table[ALPHABET_SIZE])
{
    for (int i = 0; i < ALPHABET_SIZE; i++)
        table[i] = m;

    for (int i = 0; i < m - 1; i++)
        table[(unsigned char)pattern[i]] = m - 1 - i;
}

/* Print the shift table (only non-default entries) */
void printShiftTable(const char *pattern)
{
    int m = (int)strlen(pattern);
    int table[ALPHABET_SIZE];
    buildShiftTable(pattern, m, table);

    printf("  Shift Table for pattern \"%s\" (default shift = %d):\n",
           pattern, m);
    printf("  %-10s %-6s\n", "Char", "Shift");
    printf("  %-10s %-6s\n", "----------", "------");

    for (int i = 32; i < 127; i++)
    {
        if (table[i] != m)
            printf("  '%-9c %-6d\n", (char)i, table[i]);
    }
    printf("\n");
}

/* Return index of first match, or -1 */
int horspoolSearch(const char *text, const char *pattern)
{
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0) return 0;
    if (m > n)  return -1;

    int table[ALPHABET_SIZE];
    buildShiftTable(pattern, m, table);

    int i = m - 1;
    while (i < n)
    {
        int k = 0;
        while (k < m && pattern[m - 1 - k] == text[i - k])
            k++;

        if (k == m)
            return i - m + 1;

        i += table[(unsigned char)text[i]];
    }
    return -1;
}

/* Return count of all matches; positions stored in matches[] */
int horspoolSearchAll(const char *text, const char *pattern,
                      int matches[], int maxMatches)
{
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    int count = 0;

    if (m == 0 || m > n) return 0;

    int table[ALPHABET_SIZE];
    buildShiftTable(pattern, m, table);

    int i = m - 1;
    while (i < n && count < maxMatches)
    {
        int k = 0;
        while (k < m && pattern[m - 1 - k] == text[i - k])
            k++;

        if (k == m)
        {
            matches[count++] = i - m + 1;
            i++;
        }
        else
        {
            i += table[(unsigned char)text[i]];
        }
    }
    return count;
}
