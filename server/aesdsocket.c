#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define PORT "9000"
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUF_SIZE 1024

static volatile sig_atomic_t exit_requested = 0;

static void signal_handler(int signo)
{
    (void)signo;
    exit_requested = 1;
}

static int setup_signals(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    /* No SA_RESTART so accept()/recv() return EINTR on signal */
    if (sigaction(SIGINT, &sa, NULL) != 0 || sigaction(SIGTERM, &sa, NULL) != 0) {
        syslog(LOG_ERR, "sigaction failed: %s", strerror(errno));
        return -1;
    }
    return 0;
}

static int open_server_socket(void)
{
    struct addrinfo hints, *res;
    int sockfd, yes = 1, rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(NULL, PORT, &hints, &res);
    if (rc != 0) {
        syslog(LOG_ERR, "getaddrinfo failed: %s", gai_strerror(rc));
        return -1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        syslog(LOG_ERR, "socket failed: %s", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
        syslog(LOG_ERR, "setsockopt failed: %s", strerror(errno));
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        syslog(LOG_ERR, "bind failed: %s", strerror(errno));
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);
    return sockfd;
}

static int daemonize(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "fork failed: %s", strerror(errno));
        return -1;
    }
    if (pid > 0)
        exit(EXIT_SUCCESS); /* parent */

    if (setsid() == -1) {
        syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
        return -1;
    }
    if (chdir("/") != 0) {
        syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
        return -1;
    }

    int devnull = open("/dev/null", O_RDWR);
    if (devnull != -1) {
        dup2(devnull, STDIN_FILENO);
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
        if (devnull > STDERR_FILENO)
            close(devnull);
    }
    return 0;
}

/* Send the whole data file back to the client, in chunks (file may exceed RAM). */
static int send_file_contents(int clientfd)
{
    char buf[BUF_SIZE];
    ssize_t n;
    int fd = open(DATA_FILE, O_RDONLY);
    if (fd == -1) {
        syslog(LOG_ERR, "open for read failed: %s", strerror(errno));
        return -1;
    }

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t sent = send(clientfd, buf + off, n - off, 0);
            if (sent == -1) {
                if (errno == EINTR)
                    continue;
                syslog(LOG_ERR, "send failed: %s", strerror(errno));
                close(fd);
                return -1;
            }
            off += sent;
        }
    }
    close(fd);
    return n < 0 ? -1 : 0;
}

static int append_packet(const char *data, size_t len)
{
    int fd = open(DATA_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        syslog(LOG_ERR, "open for append failed: %s", strerror(errno));
        return -1;
    }
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w == -1) {
            if (errno == EINTR)
                continue;
            syslog(LOG_ERR, "write failed: %s", strerror(errno));
            close(fd);
            return -1;
        }
        off += w;
    }
    close(fd);
    return 0;
}

static void handle_client(int clientfd)
{
    char recvbuf[BUF_SIZE];
    char *packet = NULL;
    size_t packet_len = 0;
    bool discarding = false; /* set after malloc failure until next newline */

    for (;;) {
        ssize_t n = recv(clientfd, recvbuf, sizeof(recvbuf), 0);
        if (n == 0)
            break; /* client closed */
        if (n < 0) {
            if (errno == EINTR) {
                if (exit_requested)
                    break;
                continue;
            }
            syslog(LOG_ERR, "recv failed: %s", strerror(errno));
            break;
        }

        char *start = recvbuf;
        size_t remaining = (size_t)n;
        while (remaining > 0) {
            char *nl = memchr(start, '\n', remaining);
            size_t chunk = nl ? (size_t)(nl - start) + 1 : remaining;

            if (!discarding) {
                char *tmp = realloc(packet, packet_len + chunk);
                if (tmp == NULL) {
                    syslog(LOG_ERR, "malloc failed, discarding over-length packet");
                    free(packet);
                    packet = NULL;
                    packet_len = 0;
                    discarding = true;
                } else {
                    packet = tmp;
                    memcpy(packet + packet_len, start, chunk);
                    packet_len += chunk;
                }
            }

            if (nl) {
                if (!discarding) {
                    if (append_packet(packet, packet_len) == 0)
                        send_file_contents(clientfd);
                }
                free(packet);
                packet = NULL;
                packet_len = 0;
                discarding = false;
            }
            start += chunk;
            remaining -= chunk;
        }
    }
    free(packet);
}

int main(int argc, char *argv[])
{
    bool daemon_mode = false;
    int opt;

    openlog("aesdsocket", LOG_PID, LOG_USER);

    while ((opt = getopt(argc, argv, "d")) != -1) {
        if (opt == 'd') {
            daemon_mode = true;
        } else {
            fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
            closelog();
            return -1;
        }
    }

    if (setup_signals() != 0) {
        closelog();
        return -1;
    }

    int sockfd = open_server_socket();
    if (sockfd == -1) {
        closelog();
        return -1;
    }

    /* Fork only after bind has succeeded */
    if (daemon_mode && daemonize() != 0) {
        close(sockfd);
        closelog();
        return -1;
    }

    if (listen(sockfd, 10) != 0) {
        syslog(LOG_ERR, "listen failed: %s", strerror(errno));
        close(sockfd);
        closelog();
        return -1;
    }

    while (!exit_requested) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
        if (clientfd == -1) {
            if (errno == EINTR)
                continue;
            syslog(LOG_ERR, "accept failed: %s", strerror(errno));
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        syslog(LOG_INFO, "Accepted connection from %s", ip);

        handle_client(clientfd);

        close(clientfd);
        syslog(LOG_INFO, "Closed connection from %s", ip);
    }

    syslog(LOG_INFO, "Caught signal, exiting");
    close(sockfd);
    remove(DATA_FILE);
    closelog();
    return 0;
}
