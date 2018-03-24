// #include <iostream>
#include <stdio.h>
int main(int argc, char **argv)
{
    char *words[] = {"hello", "world", "i", "am", "daydream"};

    char **p = words;
    p++;
    printf("%s\n", *p);

    return 0;
}