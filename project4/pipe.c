#include <stdio.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <string.h> 

int main() 
{
    int pip[2];
    if(pipe(pip) == -1)
    {
        fprintf(stderr, "Pipe error: %s\n ", strerror(errno));
        return -1; 
    }

    fcntl(pip[1], F_SETFL, O_NONBLOCK);
    int count = 0;
    char* buf[256]; 

    while(write(pip[1], buf, 256) != -1)
        {++count;}
    if(errno == EAGAIN)
        {printf("Pipe Capacity: %d\n", count * 256);}
    else
        {fprintf(stderr, "ERROR: errno not equal to EAGAIN => %s\n", strerror(errno));}

    return 0; 
}
