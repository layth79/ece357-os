#include <stdio.h>

int data = 6; 
int bss; 
int main()
{
    int stack = 69; 
    printf("text: %p\ndata: %p\nbss: %p\nstack: %p\n", &main, &data, &bss, &stack);
    return 0; 
}
