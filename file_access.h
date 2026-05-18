#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

/* Sequential access: read and print every line top-to-bottom */
void sequentialAccess(char filename[]);

/* Direct access: jump instantly to a single line via byte-offset index */
void directAccess(char filename[], int targetLine);

/* Direct access demo: jump to multiple flagged lines */
void directAccessDemo(char filename[], int lines[], int count);

#endif