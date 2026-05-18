#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
  #include <io.h>
  #include <direct.h>
#else
  #include <dirent.h>
#endif

#include "horspool.h"
#include "naive.h"
#include "scanner.h"
#include "file_access.h"
#include "html_template.h"

#define MAX_PATTERNS  50
#define PATTERN_LEN   50
#define MAX_FILES     200
#define FILE_LEN      512
#define MAX_FLAGGED   200

/* ================================================================
   Load patterns from file, one per line
   ================================================================ */
int loadPatterns(const char *filename, char patterns[][PATTERN_LEN])
{
    FILE *fp = fopen(filename, "r");
    if (!fp) { printf("[!] Cannot open pattern file: %s\n", filename); return 0; }

    int count = 0;
    while (count < MAX_PATTERNS && fgets(patterns[count], PATTERN_LEN, fp))
    {
        int len = (int)strlen(patterns[count]);
        while (len > 0 && (patterns[count][len-1]=='\n'||patterns[count][len-1]=='\r'))
            patterns[count][--len] = '\0';
        if (len > 0) count++;
    }
    fclose(fp);
    return count;
}

/* ================================================================
   Collect source files (.c .h .py .java) from a directory
   ================================================================ */
int collectFiles(const char *dir, char files[][FILE_LEN], int maxFiles)
{
    int count = 0;

#ifdef _WIN32
    char pattern[FILE_LEN];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);

    struct _finddata_t fd;
    intptr_t handle = _findfirst(pattern, &fd);
    if (handle == -1L) return 0;

    do {
        if (fd.name[0] == '.') continue;

        char path[FILE_LEN];
        snprintf(path, sizeof(path), "%s\\%s", dir, fd.name);

        if (fd.attrib & _A_SUBDIR)
        {
            /* Skip test_files to avoid scanning the generated benchmark */
            if (strcmp(fd.name, "test_files") == 0) continue;
            count += collectFiles(path, files + count, maxFiles - count);
        }
        else
        {
            char *dot = strrchr(fd.name, '.');
            if (dot && (strcmp(dot,".c")==0 || strcmp(dot,".h")==0  ||
                        strcmp(dot,".py")==0|| strcmp(dot,".java")==0))
            {
                if (count < maxFiles)
                    strncpy(files[count++], path, FILE_LEN - 1);
            }
        }
    } while (_findnext(handle, &fd) == 0 && count < maxFiles);

    _findclose(handle);
#else
    DIR *d = opendir(dir);
    if (!d) return 0;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL && count < maxFiles)
    {
        if (entry->d_name[0] == '.') continue;
        if (strcmp(entry->d_name, "test_files") == 0) continue;

        char path[FILE_LEN];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (stat(path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode))
            count += collectFiles(path, files + count, maxFiles - count);
        else
        {
            char *dot = strrchr(entry->d_name, '.');
            if (dot && (strcmp(dot,".c")==0 || strcmp(dot,".h")==0  ||
                        strcmp(dot,".py")==0|| strcmp(dot,".java")==0))
                strncpy(files[count++], path, FILE_LEN - 1);
        }
    }
    closedir(d);
#endif

    return count;
}

/* ================================================================
   Performance benchmark: Horspool vs Naive
   Generates a 10,000-line dataset directly in memory to compare
   Horspool vs. Naive algorithm performance on large datasets
   without cluttering the disk.
   ================================================================ */
void runBenchmark(const char *pat)
{
    const char *lines[] = {
        "    int x = 0;",
        "    char buf[256];",
        "    printf(\"value: %d\\n\", x);",
        "    for (int i = 0; i < 100; i++) x += i;",
        "    strcpy(buf, \"data\");",           /* banned */
        "    /* TODO: refactor this block */",  /* banned */
        "    if (x > 0) return x;",
        "    char *password = \"hunter2\";",    /* banned */
        "    goto cleanup;",                    /* banned */
        "    eval(userInput);",                 /* banned */
        "    memset(buf, 0, sizeof(buf));",
        "    return 0;",
    };
    int n = (int)(sizeof(lines) / sizeof(lines[0]));

    long capacity = 600000;
    char *buf = (char *)malloc(capacity);
    if (!buf) return;

    buf[0] = '\0';
    long offset = 0;
    offset += snprintf(buf + offset, capacity - offset, "/* Auto-generated benchmark dataset */\n#include <stdio.h>\n#include <string.h>\n\n");

    for (int i = 0; i < 10000; i++)
    {
        if (i % 15 == 0)
            offset += snprintf(buf + offset, capacity - offset, "void func_%d() {\n", i / 15);
        offset += snprintf(buf + offset, capacity - offset, "%s\n", lines[i % n]);
        if (i % 15 == 14)
            offset += snprintf(buf + offset, capacity - offset, "}\n\n");
    }

    int matches[2000];

    clock_t t0 = clock();
    int hc = horspoolSearchAll(buf, pat, matches, 2000);
    clock_t t1 = clock();
    int nc = naiveSearchAll(buf, pat, matches, 2000);
    clock_t t2 = clock();

    double hms = (double)(t1 - t0) / CLOCKS_PER_SEC * 1000.0;
    double nms = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000.0;

    printf("\n  +-----------------------+---------------+---------------+\n");
    printf(  "  | Algorithm             | Matches Found | Time (ms)     |\n");
    printf(  "  +-----------------------+---------------+---------------+\n");
    printf(  "  | Horspool              | %-13d | %-13.4f |\n", hc, hms);
    printf(  "  | Naive                 | %-13d | %-13.4f |\n", nc, nms);
    printf(  "  +-----------------------+---------------+---------------+\n");

    if (hms > 0 && nms > hms)
        printf("  Horspool is %.2fx faster than Naive.\n\n", nms / hms);
    else
        printf("  Both completed in similar time on this input.\n\n");

    /* Append to report */
    FILE *rep = fopen("report.txt", "a");
    if (rep)
    {
        fprintf(rep, "\n======== PERFORMANCE BENCHMARK ========\n");
        fprintf(rep, "Dataset Size : 10,000 lines (~%ld bytes in memory)\n", offset);
        fprintf(rep, "Pattern      : \"%s\"\n\n", pat);
        fprintf(rep, "%-22s %-10s %-12s\n", "Algorithm", "Matches", "Time(ms)");
        fprintf(rep, "%-22s %-10d %-12.4f\n", "Horspool",  hc, hms);
        fprintf(rep, "%-22s %-10d %-12.4f\n", "Naive",     nc, nms);
        fprintf(rep, "========================================\n\n");
        fclose(rep);
    }

    free(buf);
}

/* ================================================================
   Similarity score — character trigram Jaccard index
   ================================================================ */
#define HSIZE 65536

static unsigned int tHash(char a, char b, char c)
{
    return ((unsigned char)a*31*31 +
            (unsigned char)b*31   +
            (unsigned char)c) % HSIZE;
}

double similarityScore(const char *f1, const char *f2)
{
    FILE *fp1 = fopen(f1, "r");
    FILE *fp2 = fopen(f2, "r");
    if (!fp1 || !fp2) { if(fp1)fclose(fp1); if(fp2)fclose(fp2); return -1.0; }

    fseek(fp1, 0, SEEK_END); long s1 = ftell(fp1); rewind(fp1);
    fseek(fp2, 0, SEEK_END); long s2 = ftell(fp2); rewind(fp2);

    char *b1 = malloc(s1 + 1);
    char *b2 = malloc(s2 + 1);
    fread(b1, 1, s1, fp1); b1[s1] = '\0';
    fread(b2, 1, s2, fp2); b2[s2] = '\0';
    fclose(fp1); fclose(fp2);

    /* Bit arrays for trigram sets */
    unsigned char set1[HSIZE/8+1]; memset(set1, 0, sizeof(set1));
    unsigned char set2[HSIZE/8+1]; memset(set2, 0, sizeof(set2));

    for (long i = 0; i+2 < s1; i++)
    { unsigned int h=tHash(b1[i],b1[i+1],b1[i+2]); set1[h/8]|=(1<<(h%8)); }
    for (long i = 0; i+2 < s2; i++)
    { unsigned int h=tHash(b2[i],b2[i+1],b2[i+2]); set2[h/8]|=(1<<(h%8)); }

    int inter=0, uni=0;
    for (int i = 0; i < HSIZE; i++)
    {
        int a=(set1[i/8]>>(i%8))&1, b=(set2[i/8]>>(i%8))&1;
        if (a&&b) inter++;
        if (a||b) uni++;
    }

    free(b1); free(b2);
    return uni==0 ? 0.0 : (double)inter/uni*100.0;
}

/* ================================================================
   Report header / footer
   ================================================================ */
void writeReportHeader(int fileCount, char files[][FILE_LEN])
{
    FILE *rep = fopen("report.txt", "w");
    if (!rep) return;

    time_t t = time(NULL);
    fprintf(rep,
        "╔══════════════════════════════════════════════╗\n"
        "║        CodeLens — Code Review Report         ║\n"
        "╚══════════════════════════════════════════════╝\n\n");
    fprintf(rep, "Generated     : %s", ctime(&t));
    fprintf(rep, "Files Scanned : %d\n\nFiles:\n", fileCount);
    for (int i = 0; i < fileCount; i++)
        fprintf(rep, "  [%d] %s\n", i+1, files[i]);
    fprintf(rep, "\n");
    fclose(rep);
}

void writeReportFooter(int total)
{
    FILE *rep = fopen("report.txt", "a");
    if (!rep) return;
    fprintf(rep,
        "\n╔══════════════════════════════════════════════╗\n"
        "║  Total Violations Found : %-4d               ║\n"
        "╚══════════════════════════════════════════════╝\n", total);
    fclose(rep);
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║        CodeLens — Code Review Tool           ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    /* --- 1. Load patterns --- */
    char patterns[MAX_PATTERNS][PATTERN_LEN];
    int  patternCount = loadPatterns("pattern.txt", patterns);

    if (patternCount == 0)
    {
        printf("[!] No patterns loaded. Check pattern.txt\n");
        return 1;
    }

    printf("[+] Patterns loaded (%d):\n", patternCount);
    for (int i = 0; i < patternCount; i++)
        printf("      [%d] \"%s\"\n", i+1, patterns[i]);
    printf("\n");

    /* --- 2. Print bad-character shift tables (DAA demo) --- */
    printf("[+] Horspool Bad-Character Shift Tables:\n\n");
    for (int i = 0; i < patternCount; i++)
        printShiftTable(patterns[i]);

    /* --- 4. Collect source files from test_files/ only --- */
    char files[MAX_FILES][FILE_LEN];
    int  fileCount = collectFiles("test_files", files, MAX_FILES);

    printf("[+] Source files found: %d\n\n", fileCount);

    /* --- 5. Write report header --- */
    writeReportHeader(fileCount, files);

    /* --- 6. Scan all files (sequential access) --- */
    int totalViolations = 0;
    int flaggedLines[MAX_FLAGGED];
    int flaggedCount   = 0;
    char firstFlagged[FILE_LEN] = "";

    printf("[+] Scanning files...\n\n");

    for (int i = 0; i < fileCount; i++)
    {
        printf("  [%d/%d] %s\n", i+1, fileCount, files[i]);

        int localLines[100];
        int localCount = 0;

        int v = scanFile(files[i], patterns, patternCount,
                         localLines, &localCount);
        totalViolations += v;

        if (v > 0)
        {
            printf("        -> %d violation(s)\n", v);
            if (firstFlagged[0] == '\0')
            {
                strncpy(firstFlagged, files[i], FILE_LEN - 1);
                for (int k = 0; k < localCount && flaggedCount < MAX_FLAGGED; k++)
                    flaggedLines[flaggedCount++] = localLines[k];
            }
        }
        else
            printf("        -> Clean\n");
    }

    printf("\n[+] Total violations: %d\n", totalViolations);

    /* --- 7. Sequential access demo (first 10 lines of first flagged file) --- */
    printf("\n[+] Sequential Access Demo:\n");
    if (firstFlagged[0] != '\0')
        sequentialAccess(firstFlagged);
    else
        printf("    (no violations found)\n");

    /* --- 8. Direct access demo (jump to flagged lines) --- */
    printf("[+] Direct Access Demo:\n");
    if (firstFlagged[0] != '\0' && flaggedCount > 0)
        directAccessDemo(firstFlagged, flaggedLines, flaggedCount);
    else
        printf("    (no flagged lines to demo)\n\n");

    /* --- 9. Performance benchmark --- */
    printf("[+] Performance Benchmark (Horspool vs Naive on large file):\n");
    runBenchmark(patterns[0]);

    /* --- 10. Similarity score (bonus) — compare only test_files/ --- */
    {
        char testDir[] = "test_files";
        char tfiles[MAX_FILES][FILE_LEN];
        int  tcount = collectFiles(testDir, tfiles, MAX_FILES);

        printf("[+] Copy-Paste Similarity Scores (test_files only):\n\n");

        FILE *rep = fopen("report.txt", "a");
        if (rep) fprintf(rep, "\n======== SIMILARITY SCORES (test_files) ========\n");

        if (tcount < 2)
        {
            printf("    (need at least 2 files in test_files/ to compare)\n\n");
            if (rep) { fprintf(rep, "  Not enough files.\n"); fclose(rep); }
        }
        else
        {
            for (int i = 0; i < tcount; i++)
            {
                for (int j = i+1; j < tcount; j++)
                {
                    double score = similarityScore(tfiles[i], tfiles[j]);
                    if (score < 0) continue;

                    /* Extract basenames for cleaner display */
                    char *n1 = strrchr(tfiles[i], '\\');
                    char *n2 = strrchr(tfiles[j], '\\');
                    if (!n1) n1 = strrchr(tfiles[i], '/');
                    if (!n2) n2 = strrchr(tfiles[j], '/');
                    n1 = n1 ? n1+1 : tfiles[i];
                    n2 = n2 ? n2+1 : tfiles[j];

                    const char *flag = (score >= 60.0) ? "  *** HIGH — possible copy-paste ***" : "";
                    printf("  %-20s <-> %-20s  %.1f%%%s\n", n1, n2, score, flag);

                    if (rep)
                        fprintf(rep, "%.1f%%  %-20s  <->  %-20s%s\n",
                                score, n1, n2, flag);
                }
            }
            printf("\n");
            if (rep) fprintf(rep, "=================================================\n");
            if (rep) fclose(rep);
        }
    }

    /* --- 11. Report footer --- */
    writeReportFooter(totalViolations);

    /* --- 12. Generate HTML interactive dashboard from report.txt --- */
    {
        FILE *txt = fopen("report.txt", "r");
        FILE *html = fopen("report.html", "w");
        if (txt && html)
        {
            fprintf(html, "%s", HTML_TEMPLATE_PART_1);
            char line[1024];
            while (fgets(line, sizeof(line), txt))
            {
                for (int i = 0; line[i] != '\0'; i++)
                {
                    if (line[i] == '<') fprintf(html, "&lt;");
                    else if (line[i] == '>') fprintf(html, "&gt;");
                    else if (line[i] == '&') fprintf(html, "&amp;");
                    else fputc(line[i], html);
                }
            }
            fprintf(html, "%s", HTML_TEMPLATE_PART_2);
            fclose(txt);
            fclose(html);
            printf("[+] Interactive dashboard written to report.html\n");
        }
        else
        {
            if (txt) fclose(txt);
            if (html) fclose(html);
            printf("[!] Failed to generate report.html\n");
        }
    }

    printf("[+] Report written to report.txt\n");
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║              Scan Complete!                  ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    return 0;
}
