#include <stdio.h>
#include <string.h>

int cp(char **args) {
    printf("Copy\n");
    return 0;
}

int mv(char **args) {
    printf("Move\n");
    return 0;
}

int rm(char **args) {
    printf("Remove\n");
    return 0;
}

int mkdir(char **args) {
    printf("Make directory\n");
    return 0;
}

int rmdir(char **args) {
    printf("Remove directory\n");
    return 0;
}

int ls(char **args) {
    printf("List files\n");
    return 0;
}

int cat(char **args) {
    printf("Cat std output\n");
    return 0;
}

int cd(char **args) {
    printf("Change directory\n");
    return 0;
}

int pwd(char **args) {
    printf("Print working directory\n");
    return 0;
}

int info(char **args) {
    printf("Info\n");
    return 0;
}

int incp(char **args) {
    printf("Copy in\n");
    return 0;
}

int outcp(char **args) {
    printf("Copy out\n");
    return 0;
}

int load(char **args) {
    printf("Load\n");
    return 0;
}

int format(char **args) {
    printf("Format\n");
    return 0;
}

int defrag(char **args) {
    printf("Defragmentation\n");
    return 0;
}

int main() {
    char input[100];
    const char delim[2] = " ";
    char *token;

    char *known_cmds[15] = {
            "cp",
            "mv",
            "rm",
            "mkdir",
            "rmdir",
            "ls",
            "cat",
            "cd",
            "pwd",
            "info",
            "incp",
            "outcp",
            "load",
            "format",
            "defrag"
    };
    int (*cmd_call[]) (char **args) = {
            cp,
            mv,
            rm,
            mkdir,
            rmdir,
            ls,
            cat,
            cd,
            pwd,
            info,
            incp,
            outcp,
            load,
            format,
            defrag
    };

    scanf("%[^\n]s", input);

    token = strtok(input, delim);

    int call = 0;
    for (int i = 0; i < 15; i++) {
        if (!strcmp(token, known_cmds[i])) {
            cmd_call[i](NULL);
            call = 1;
        }
    }
    if (!call) printf("UNKNOWN COMMAND!\n");

    while (token) {
//        printf("%s\n", token);
        token = strtok(NULL, delim);
    }

    return 0;
}
