#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    openlog("writer", LOG_PID, LOG_USER);
    if (argc != 3){
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        fprintf(stderr, "Error: Two parameters required\n");
        fprintf(stderr, "Usage: %s <writefile> <writestr>\n", argv[0]);
        closelog();
        return 1;
    }
    const char *writefile = argv[1];
    const char *writestr = argv[2];
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
    FILE *file = fopen(writefile, "w");
    if (file == NULL){
        syslog(LOG_ERR, "Could not open file %s for writing: %s", writefile, writestr);
        fprintf(stderr, "Error: Could not create file %s", writefile);
        closelog();
        return 1;
    }
    if (fprintf(file, "%s", writestr) < 0){
        syslog(LOG_ERR, "Error writing to file %s: %s", writefile, strerror(errno));
        fprintf(stderr, "Error: Could not write to file %s\n", writefile);
        fclose(file);
        closelog();
        return 1;
    }
    if (fclose(file) !=0){
        syslog(LOG_ERR, "Error closing file %s: %s", writefile, strerror(errno));
        fprintf(stderr, "Error: Could not close file %s\n", writefile);
        closelog();
        return 1;
    }
    closelog();
    return 0;

}