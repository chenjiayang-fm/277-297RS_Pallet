#include "rwrf.h"

int master_id = 0xA0;
int slave_id = 0xB0;
unsigned short port = 1234;

void master(void)
{
    rf_master_init(master_id);

    rf_set_remote_id(&slave_id, 1);

    /* Create datagram socket */
    int sockfd = rf_socket(RF_SOCK_DGRAM);

    /* Bind local port */
    int result = rf_bind(sockfd, port);

    /* Receive */
    char buf[1024] = {0};
    size_t buf_len = sizeof(buf);
    while (1) {
        struct rfaddr addr;
        int result = rf_recvfrom(sockfd, buf, buf_len, 0, &addr);
        if (result > 0) {
            printf("Received %d bytes from %#x:%hu    \"%s\"\n", result, addr.id, addr.port, buf);
        } else {
            /* Error */
        }
    }

    /* Deallocate socket when no longer used */
    rf_close(sockfd);

    /* Terminate library */
    rf_cleanup();
}

void slave(void)
{
    rf_slave_init(slave_id);

    rf_set_remote_id(&master_id, 1);

    /* Create datagram socket */
    int sockfd = rf_socket(RF_SOCK_DGRAM);

    /* Setup destination address (id and port) */
    struct rfaddr addr;
    addr.id = master_id;
    addr.port = port;

    /* Send something */
    char buf[1024];
    int n;
    for (n = 1; 1; n++) {
        int byte_printed = snprintf(buf, sizeof(buf), "Hello World %d", n);
        size_t len = byte_printed > 0 ? byte_printed : 0;
        int result = rf_sendto(sockfd, buf, len, 0, &addr);
        if (result > 0) {
            printf("Sent %d bytes to %#x:%hu    \"%s\"\n", len, addr.id, addr.port, buf);
        } else {
            /* Error */
        }
    }

    /* Deallocate socket when no longer used */
    rf_close(sockfd);

    /* Terminate library */
    rf_cleanup();
}

void simple_test_thread(void const *arg)
{
#ifdef VBM_PU
    master();
#else
    slave();
#endif
}
