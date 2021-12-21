#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <stdint.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>

int recursiveHelper(char *parent_path, char *cur_dir);
int length = 8;  //to dynamically change the spacing for long uids

int main(int argc, char *argv[])
{
    char* startDir = ".";
    if (argc > 1 && strcmp("./", argv[1])) //remove the slash becuz we add it later
    {
        int len = strlen(argv[1]);
        if (argv[1][len - 1] == '/' && strcmp(argv[1], "/") != 0)
        {

            startDir = argv[1];
            startDir[strlen(startDir) - 1] = '\0'; 
        }
        else 
            startDir = argv[1]; 
    }
    if (argc > 2) 
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    recursiveHelper(startDir, ""); //start the recursion
    return 0;
}

int recursiveHelper(char *parent_path, char *cur_dir) 
{
    if (!cur_dir) //base case 
        return 0; 
    char path[1024];  //path limit is 1024 characters
    if (cur_dir != "") //this is the initial run 
        snprintf(path, sizeof(path), "%s/%s", parent_path, cur_dir);
    else
        strcpy(path, parent_path);
    
    DIR* dirStream = opendir(path); //open the stream 
    if (!dirStream)
    {
        fprintf(stderr, "Failed to open directory stream %s: %s\n", path, strerror(errno));
        return -1;
    }

    errno = 0; 
    struct dirent* dirEnt = readdir(dirStream); //read the first item from the stream 
    while(dirEnt)
    {
        if (strcmp(dirEnt->d_name, "..") == 0) //if previous dir, get the next item in stream and continue
        {
            errno = 0;
            dirEnt = readdir(dirStream);
            if (!dirEnt->d_name && errno != 0)
            {
                fprintf(stderr, "Failed to attain value from directory stream %s: %s\n", path, strerror(errno));
                return -1;
            }
            continue;
        }
        if (dirEnt->d_type == DT_DIR && strcmp(".", dirEnt->d_name) != 0) //if a directory, recurse after getting next item in steram
        {
            recursiveHelper(path, dirEnt->d_name);
            errno = 0;
            dirEnt = readdir(dirStream);
            if (!dirEnt->d_name && errno != 0)
            {
                fprintf(stderr, "Failed to attain value from directory stream %s: %s\n", path, strerror(errno));
                return -1;
            }
            continue;
        }

        struct stat sb;
        char new_path[strlen(path) + strlen(dirEnt->d_name) + 1];
        strcpy(new_path, path); 
        strcat(new_path, "/");
        strcat(new_path, dirEnt->d_name); //get the new path which represents what we're currently on 
        if (lstat(new_path, &sb) == -1)
        {
            fprintf(stderr,"%s: %s", new_path, strerror(errno));
            return -1; 
        }

        printf("%9ld ", (long) sb.st_ino); //print inode number
        printf("%6lld ", (long long) sb.st_blocks/2); //print number of blocks 

        int flag = 0;                
        switch (sb.st_mode & S_IFMT) { //retrieve the type and set flags for char and block devices and links 
            case S_IFBLK:  printf("b"); flag = 2; break;
            case S_IFCHR:  printf("c"); flag = 2; break;
            case S_IFDIR:  printf("d");        break;
            case S_IFIFO:  printf("p");        break;
            case S_IFLNK:  printf("l"); flag = 1; break;
            case S_IFREG:  printf("-");        break;
            case S_IFSOCK: printf("s");        break;
            default:       printf("?");        break;
        }
        // code based on https://opensource.apple.com/source/Libc/Libc-167/string.subproj/strmode.c.auto.html
        char p[10]; //get the permissions
        if (sb.st_mode & S_IRUSR) p[0] = 'r'; else p[0] = '-';
        if (sb.st_mode & S_IWUSR) p[1] = 'w'; else p[1] = '-';
        switch (sb.st_mode & (S_IXUSR | S_ISUID)) {
        case 0:
            p[2] = '-';
            break;
        case S_IXUSR:
            p[2] = 'x';
            break;
        case S_ISUID:
            p[2] = 'S';
            break;
        case S_IXUSR | S_ISUID:
            p[2] = 's';
            break;
        }
        if (sb.st_mode & S_IRGRP) p[3] = 'r'; else p[3] = '-';
        if (sb.st_mode & S_IWGRP) p[4] = 'w'; else p[4] = '-';
        switch (sb.st_mode & (S_IXGRP | S_ISGID)) {
        case 0:
            p[5] = '-';
            break;
        case S_IXGRP:
            p[5] = 'x';
            break;
        case S_ISGID:
            p[5] = 'S';
            break;
        case S_IXGRP | S_ISGID:
            p[5] = 's';
            break;
        }
        if (sb.st_mode & S_IROTH) p[6] = 'r'; else p[6] = '-';
        if (sb.st_mode & S_IWOTH) p[7] = 'w'; else p[7] = '-';
        switch (sb.st_mode & (S_IXOTH | S_ISVTX)) {
        case 0:
            p[8] = '-';
            break;
        case S_IXOTH:
            p[8] = 'x';
            break;
        case S_ISVTX:
            p[8] = 'T';
            break;
        case S_IXOTH | S_ISVTX:
            p[8] = 't';
            break;
        }    

        printf("%s ", p); //print permisison
        printf("%3ld ", (long) sb.st_nlink); //print number of links 
        struct passwd *pwd, *pwdg;
        char *uid; 
        char *gid;
        int cur_len; 
        if ((pwd = getpwuid(sb.st_uid)) == NULL) //print UID number or char value if you can 
        {
            if(errno)
            {
                fprintf(stderr, "error getting user id for %s: %s\n", dirEnt->d_name, strerror(errno));
                return -1;
            } 
            else
                printf("%d ", sb.st_uid);
        }
        else
        {
            cur_len = strlen(pwd->pw_name); 
            if (length < cur_len)
                length = cur_len; 
            printf("%-*s ", length,  pwd->pw_name);
        }
        
        if ((pwdg = getpwuid(sb.st_gid)) == NULL) //print GID number or fchar value if you can 
        {
            if(errno)
            {
                fprintf(stderr, "error getting user id for %s: %s\n", dirEnt->d_name, strerror(errno));
                return -1;
            } 
            else
                printf("%-4d ", sb.st_gid);

        }
        else 
        {
            printf("%-4s ", pwdg->pw_name);
        }
	
        if(flag == 2) //if block device or char device, print major and minor device numbers
        {
            unsigned int major = major(sb.st_rdev); 
            unsigned int minor = minor(sb.st_rdev); 
            printf("%7u,", major);
            printf(" %u ", minor);
        }
        else
            printf("%12lld ", (long long) sb.st_size); //otherwise print size 
        char buff[70];
        struct tm *t_point = localtime(&sb.st_mtime); //print time 
        strftime(buff, sizeof buff, "%b %e  %Y", t_point); 
        printf("%s ", buff);
        if (strcmp(dirEnt->d_name, ".") == 0) //if it's the current directory print it's value
        {
            printf("%s\n", path);
        }
        else
        {
            if(flag == 1) //for links 
            {
                char buffer[1024]; 
                readlink(new_path, buffer, 1024);
                printf("%s -> %s\n", new_path, buffer);
            }
            else 
            {
                printf("%s\n", new_path); //for regular thing
            }
        }
        
        errno = 0;
        dirEnt = readdir(dirStream); //get next item in the directory stream 
        if (!dirEnt->d_name && errno != 0)
        {
            fprintf(stderr, "Failed to attain value from directory stream %s: %s\n", path, strerror(errno));
            return -1;
        }
    }
    if (errno != 0)
    {
        fprintf(stderr, "Failed to attain value from directory stream %s: %s\n", path, strerror(errno));
        return -1;
    }
    return 1;
}
