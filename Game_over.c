#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int main(int argc, char const *argv[]) {
    int number = atoi(argv[1]);

    printf(ANSI_COLOR_CYAN "\n\n\nCONGRATULATIONS! The player %d won\n\n\n", number);
    printf(ANSI_COLOR_RESET "\n");
    sleep(5);


    return 0;
}