// Layth Yassin, Ali Ghuman
// ECE-357
// Pset #1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int opt, flags, fd, idx, firstEl = 1, numArgs, bytes = 0, writes = 0, reads = 0, binary_flag = 0, binary_file_num;
    char* fname = argv[0];
    char buf[4096];

    // parses command input by user, determine what the infile and outfiles are and if command is valid
    opt = getopt(argc, argv, "-o");
    switch (opt) {
        // outfile flag present
        case 'o':
            flags = 1;
            break;
        // outfile flag not present
        case 1:
        case -1:
            flags = 2;
            break;
        default:
            fprintf(stderr, "USAGE:\n\tkitty [-o outfile] infile1 [...infile2....]\n\tkitty [-o outfile]\n");
            return -1;
    }
 
    // open outfile if outfile flag present
    if (flags == 1) {
        fd = open(argv[2], O_WRONLY|O_APPEND|O_TRUNC|O_CREAT, 0666);
        if (fd == -1) {
            fprintf(stderr, "ERROR: Failed to open/create %s: %s\n", argv[2], strerror(errno));
            return -1;
        }
        // start index at 3 since there are at least 3 arguments in command
        idx = 3;
        numArgs = argc - 3;
    }
    else if (flags == 2) {
        // start index at 1 since there is at least 1 argument in command
        idx = 1;
        numArgs = argc - 1;
    }
    // if there are args remaining, or this is the first time entering the loop, proceed
    while (numArgs > 0 || firstEl) {
        int fd_infile;
        if ((firstEl && numArgs == 0) || strcmp(argv[idx], "-") == 0) { //check for stdin
            fd_infile = 0;
        }
        else {
            fd_infile = open(argv[idx], O_RDONLY);
        }
        firstEl = 0;
        if (fd_infile == -1) {
            fprintf(stderr, "ERROR: Could not open %s for reading: %s\n", argv[idx], strerror(errno));
            return -1;
        }

        int ret;
        while ((ret = read(fd_infile, buf, 4096)) > 0) {
            if (binary_flag == 0) {
                for (int j = 0; j < ret; j++) {
                    if ((buf[j] > 0 & buf[j] < 9) || (buf[j] >= 127) || (buf[j] > 13 & buf[j] < 32)) { //check for binary
                        binary_flag = 1;
                        if (fd_infile == 0) {
                            fprintf(stderr, "WARNING: <standard input> is a *BINARY FILE*\n");
                        }
                        else {
                            fprintf(stderr, "WARNING: %s is a *BINARY FILE*\n", argv[idx]);
                        }
                        break;
                    }
                }
            }
            ++reads;
            bytes += ret; //count number of bytes
            if (ret == -1) {
                fprintf(stderr, "ERROR: Could not read %s: %s\n", argv[idx], strerror(errno));
                return -1;
            }
            else {
                int wrt;
                if (flags == 1) { //for reading into outfile
                    wrt = write(fd, buf, ret);
                    ++writes;
                    ret -= wrt;
                    while(!(ret == 0)) { //make sure there wasn't a partial read
                        wrt = write(fd, buf + wrt, ret);
                        ret -= wrt;
                        ++writes;
                    }
                }
                else { //for reading from stdin
                    wrt = write(1, buf, ret);
                    ++writes;
                    ret -= wrt;
                    while(!(ret == 0)) {
                        wrt = write(fd, buf + wrt, ret);
                        ret -= wrt; 
                        ++writes; 
                    }
                }
                if (wrt == -1) {
                    fprintf(stderr, "ERROR: Could not write to %s: %s\n", argv[idx], strerror(errno));
                    return -1;
                }
            }
        }
        if (fd_infile){ //close the input files
            if (close(fd_infile) == -1) {
                fprintf(stderr, "ERROR: Could not close %s: %s\n", argv[idx],  strerror(errno));
                return -1;
            }
        }
        binary_flag = 0;
        ++idx;
        --numArgs;
    }

    if (flags == 1) { //close the output file
        if (close(fd) == -1) {
            fprintf(stderr, "ERROR: Could not close %s: %s\n", argv[2], strerror(errno));
            return -1;
        }
    }

    fprintf(stderr,"Bytes Transferred: %d\nRead Calls: %d\nWrite Calls: %d\n", bytes, reads, writes);

    return 0; 
}
