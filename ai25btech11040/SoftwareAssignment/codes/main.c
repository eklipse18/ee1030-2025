#include <stdio.h>


int main(int argc, const char * argv[]) {
    FILE *file;
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    fclose(file);
    return 0;
}