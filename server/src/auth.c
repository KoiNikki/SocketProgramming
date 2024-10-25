#include <stdio.h>
#include <string.h>
#include "auth.h"

int validate_user(const char *username) {
    if (strcmp(username, "anonymous") == 0) {
        return 1;
    }
    return 0;
}

int validate_password(const char *password) {
    if (strstr(password, "@") != NULL) {
        return 1;
    }
    return 0;
}
