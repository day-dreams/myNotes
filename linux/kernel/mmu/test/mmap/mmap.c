#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd = open("mmap.c", O_RDONLY);
    if (!fd)
    {
        printf("open file error\n");
        return 0;
    }

    struct stat s;
    if (fstat(fd, &s) == -1)
    {
        printf("fstat error\n");
        return -1;
    }

    int length = s.st_size;
    printf("%d\n", length);

    char* ptr = (char*)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (ptr == MAP_FAILED)
    {
        printf("mmap error\n");
        return 0;
    }

    // for (int i = 0; i != s.st_size; ++i)
    //     printf("%c", ((char*)ptr)[i]);

    printf("%c\n", ptr[0]);
    ptr[0] = '9';
    printf("%c\n", ptr[0]);

    munmap((void*)ptr, s.st_size);

    return 0;
}
