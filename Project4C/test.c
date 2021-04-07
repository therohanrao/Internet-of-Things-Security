#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
    FILE* write = fopen("buf9928.txt", "w");
    char buf[100];
    char tests[] = "test1\ntest2\ntest3\ntest4\n";
    fputs(tests, write);
    fclose(write);
    FILE* read = fopen("buf9928.txt", "r");
    while(fgets(buf, 100, read))
        printf("%s",buf);
    fclose(read);
}