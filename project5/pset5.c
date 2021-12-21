#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>


// test 1 handler
void handler(int sig)
{
    fprintf(stderr, "Signal: (%s)\n", strsignal(sig));
    _exit(sig);
}

// Test case 1 - PROT_READ Violation
int test1(int fd, char buf[15])
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    //who needs loops anyway when you have vim keybinds B)
    (void) sigaction(0,  &sa, NULL);
    (void) sigaction(1,  &sa, NULL);
    (void) sigaction(2,  &sa, NULL);
    (void) sigaction(3,  &sa, NULL);
    (void) sigaction(4,  &sa, NULL);
    (void) sigaction(5,  &sa, NULL);
    (void) sigaction(6,  &sa, NULL);
    (void) sigaction(7,  &sa, NULL);
    (void) sigaction(8,  &sa, NULL);
    (void) sigaction(9,  &sa, NULL);
    (void) sigaction(10, &sa, NULL);
    (void) sigaction(11, &sa, NULL);
    (void) sigaction(12, &sa, NULL);
    (void) sigaction(13, &sa, NULL);
    (void) sigaction(14, &sa, NULL);
    (void) sigaction(15, &sa, NULL);
    (void) sigaction(16, &sa, NULL);
    (void) sigaction(17, &sa, NULL);
    (void) sigaction(18, &sa, NULL);
    (void) sigaction(19, &sa, NULL);
    (void) sigaction(20, &sa, NULL);
    (void) sigaction(21, &sa, NULL);
    (void) sigaction(22, &sa, NULL);
    (void) sigaction(23, &sa, NULL);
    (void) sigaction(24, &sa, NULL);
    (void) sigaction(25, &sa, NULL);
    (void) sigaction(26, &sa, NULL);
    (void) sigaction(27, &sa, NULL);
    (void) sigaction(28, &sa, NULL);
    (void) sigaction(29, &sa, NULL);
    (void) sigaction(30, &sa, NULL);
    (void) sigaction(31, &sa, NULL);

    char* map = mmap(NULL, 15, PROT_READ, 0, fd, 0);
    
    printf("Contents in buffer are %s\n", buf);
    printf("Writing a x to map[0]\n");

    //try to write to the file
    map[0] = 'x';
    if(map[0] != 'x')
        return 255;

    close(fd);
    return 0; 
}

// Test case 2 - writing to MAP_SHARED
// Test case 3 - writing to MAP_PRIVATE
int test2_3(int fd, char buf[15], int permissions)
{
    char* map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, permissions, fd, 0);
    
    printf("Contents in buffer are %s\n", buf);
    printf("Writing an 'x' to map[2]\n");
    
    map[2] = 'x';
    char charRead[15];
    lseek(fd, 0, SEEK_SET);

    if ((read(fd, charRead, sizeof(charRead))) == -1)
    {
        fprintf(stderr, "ERROR: could not read: %s", strerror(errno));
        return -1;
    }

    printf("Read in from file after: %s\n", charRead);

    if(charRead[2] == 'k')
    {
        printf("Contents of the buffer did not change!\n");
        return 1; 
    }
    else if (charRead[2] != 'x') 
    {
        printf("Something went wrong which means Hak's nose is purple!\n");
    }

    printf("The file changed properly.\n"); 
    return 0; 
}

// Test case 4 - writing into hole
int test4(int fd, char buf[15])
{
    //Increase size of file to 4100 bytes long and append a byte
    lseek(fd, 4100, SEEK_SET);
    write(fd, "a", 1);
    
    // map 8192 bytes of shared memory
    char* map = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    map[4101] = 'X'; 

    //increase file sizse
    lseek(fd, 4117, SEEK_SET);
    write(fd, "a", 1);

    //seek back to check to see if x is there
    lseek(fd, 4101, SEEK_SET);
    char charRead[1]; 
    if ((read(fd, charRead, sizeof(charRead))) == -1)
    {
        fprintf(stderr, "Error while reading: %s", strerror(errno));
        return -1; 
    }

    if(strcmp(charRead, "X") != 0)
    {
        printf("X is not visible\n");
        return 1; 
    }

    printf("X is visible\n");
    return 0; 
}

int main(int argc, char *argv[])
{
    // handle input argument
    if (!argv[1]) {
        fprintf(stderr, "Invalid input\n");
        return -1; 
    }
    int numeroElTesto = atoi(argv[1]);

    int fd;
    if ((fd = open("testicle.txt", O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0)
    {
        fprintf(stderr, "ERROR: cannot open file: %s.\n", strerror(errno));
        return -1;
    }
    
    // Write string to file
    char buf[15] = "Hak's Red Nose";
    if (write(fd, buf, 15) < 0)
    {
        fprintf(stderr, "ERROR: cannot write to file: %s\n", strerror(errno));
        return -1;
    }
    
    // determine what test to run
    switch (numeroElTesto)
    {
        case 1:
            printf("Executing Test #1 (write to r/o mmap):\n");
            return test1(fd, buf);
        case 2:
            printf("Executing Test #2 (write to MAP_SHARED file)\n");
            return test2_3(fd, buf, MAP_SHARED);
        case 3:
            printf("Executing Test #3 (write to MAP_PRIVATE file)\n");
            return test2_3(fd, buf, MAP_PRIVATE);
        case 4:
            printf("Executing Test #4 (writing into a hole)\n");
            return test4(fd, buf);
        default: 
            fprintf(stderr, "ERROR: Input was not valid. Enter a '1', '2', '3', or '4'.\n");
            return -1; 
    }
}
