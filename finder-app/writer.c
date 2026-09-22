#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    openlog("writer", LOG_PID, LOG_USER);

    if (argc < 3){
        syslog(LOG_ERR, "Usage: <writefile> <writestr>");
        fprintf(stderr, "Error: 2 arguments required.\n");
        closelog();
        return 1;
    }

    const char *writefile = argv[1];
    const char *writestr = argv[2];

    FILE *fp = fopen(writefile, "wb");
    if (fp == NULL){
        syslog(LOG_ERR, "Could not open %s: %s", writefile, strerror(errno));
        closelog();
        return 1;
    }

    size_t len = strlen(writestr);
    size_t written = fwrite(writestr, 1, len, fp);

    if (written != len){
        syslog(LOG_ERR, "Failed to write full string to %s", writefile);
        fclose(fp);
        closelog();
        return 1;
    }
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    fclose(fp);
    closelog();
    return 0;

}