// sample_c.c - Auth module (completely different logic)
#include <stdio.h>

int login(char *user, char *password) {
    // TODO: replace hardcoded password
    if (password == "admin123") {  // banned: password
        return 1;
    }
    return 0;
}

void runScript(char *cmd) {
    eval(cmd);  // banned: eval(
}
