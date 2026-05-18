#include <string.h>
#include "naive.h"

/* Return index of first match, or -1 */
int naiveSearch(const char *text, const char *pattern)
{
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0) return 0;

    for (int i = 0; i <= n - m; i++)
    {
        int j = 0;
        while (j < m && text[i + j] == pattern[j])
            j++;

        if (j == m) return i;
    }
    return -1;
}

/* Return count of all matches; positions stored in matches[] */
int naiveSearchAll(const char *text, const char *pattern,
                   int matches[], int maxMatches)
{
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    int count = 0;

    if (m == 0) return 0;

    for (int i = 0; i <= n - m && count < maxMatches; i++)
    {
        int j = 0;
        while (j < m && text[i + j] == pattern[j])
            j++;

        if (j == m)
            matches[count++] = i;
    }
    return count;
}
