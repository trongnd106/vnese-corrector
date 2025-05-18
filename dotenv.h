#ifndef DOTENV_H
#define DOTENV_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void load_dotenv(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *value = eq + 1;
        
        value[strcspn(value, "\r\n")] = '\0';

        setenv(key, value, 1);
    }

    fclose(file);
}

#endif // DOTENV_H
