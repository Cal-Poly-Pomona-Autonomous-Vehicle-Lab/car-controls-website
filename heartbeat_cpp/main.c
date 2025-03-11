#include <stdio.h> 
#include <stdbool.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/time.h>
#include <sys/select.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <errno.h>

#include <netdb.h> 
#include <netinet/in.h> 
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


#define PORT 8000
#define BUFF_SIZE 2024 

bool isThreadAlive = false;

/* Computing the checksum https://www.cs.dartmouth.edu/~sergey/cs60/lab3/icmp4-rawsend.c */
uint16_t
checksum_ck (char *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += (addr[0] << 8) | (addr[1] & 0xFF);
    addr += 2;
    count -= 2;
  }

  if (count > 0)
    sum += (addr[0] & 0xFF) << 8;
  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) { 
    sum = (sum & 0xFFFF) + (sum >> 16);
  } 

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

bool check_connectivity() {
    char buffer[48]; 
    memset(&buffer, 0, 48); 

    struct timeval timeout; 
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0;

    int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_fd < 0) {
        perror("Failed to create socket"); 
        return false;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Failed to create sockopt");
        return false; 
    } 

    /* https://github.com/leostratus/netinet/blob/master/ip_icmp.h */
    struct icmp *icmp = (struct icmp*)buffer; 
    icmp->icmp_type = ICMP_ECHO; 
    icmp->icmp_code = 0; 
    icmp->icmp_cksum = 0;
    icmp->icmp_hun.ih_idseq.icd_seq = 1;
    icmp->icmp_hun.ih_idseq.icd_id = 0;

    // Buffer[8] = 'a'; 
    // Buffer[9] = 'b'; 
    // Buffer[10] = 'c'; 
    // Buffer[11] = 'd';
    // Buffer[12] = 'e'; 
    // Buffer[13] = 'f'; 
    // Buffer[14] = 'g'; 
    // Buffer[15] = 'h'; 
    // Buffer[16] = 'i'; 
    // Buffer[17] = 'j'; 
    // Buffer[18] = 'k';
    // Buffer[17] = 'l'; 
    // Buffer[18] = 'm'; 
    // Buffer[19] = 'n'; 
    // Buffer[20] = 'o'; 
    buffer[32] = 'p'; 

    time_t ts; 
    time(&ts);
    gmtime(&ts);
    icmp->icmp_dun.id_ts.its_otime = ts;
    icmp->icmp_cksum = htons(checksum_ck(buffer, sizeof(buffer)));

    struct sockaddr_in dest_addr; 
    int dest_addr_len = sizeof(dest_addr);

    dest_addr.sin_family = AF_INET; 
    inet_aton("8.8.8.8", &dest_addr.sin_addr); 
    dest_addr.sin_port = htons(443);

    struct sockaddr_in from;
    int from_len = sizeof(from);

    printf("Type: %d\n", icmp->icmp_type);
    printf("Code: %d\n\n", icmp->icmp_code); 

    // icmp->icmp_cksum = htons(0x1F1F); 

    int send_bytes = sendto(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&dest_addr, (socklen_t)dest_addr_len);
    if (send_bytes < 0){
        perror("Failed to send icmp packet"); 
        return false;
    }

    printf("Sending...\n");
    fflush(stdout);
    int bytes = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, (socklen_t *)&from_len);
    if (bytes < 0) {
        perror("Failed to recieved icmp packet");
        return false; 
    }

    struct icmp *icmp_reply = (struct icmp*)buffer + 20; 

    printf("Type: %d\n", icmp_reply->icmp_type);
    printf("Code: %u\n", icmp_reply->icmp_code);
    fflush(stdout);

    close(sock_fd);
    switch (icmp_reply->icmp_type) {
        case ICMP_ECHOREPLY:
            return true;
        case ICMP_UNREACH_ISOLATED:
            return false;
    }

    return false;
}


void *check_connection_thread(void* thread_id) {
    bool isConnected = true;
    while (isConnected) {
        printf("Thread\n");
        isConnected = check_connectivity(); 
        sleep(5);
    }

    isThreadAlive = false;
    pthread_exit(NULL);
}

int start_server() {
    int server_fd; 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE);
    }

    int opt = 1; 
    if ((setsockopt(server_fd, SOL_SOCKET, SO_RESUSEADDR, &opt, sizeof(opt))) < 0) {
        perror("Set Socket options failed: "); 
        return -1; 
    }

    return server_fd; 
}

bool wait_for_update(int fd, struct fd_set *rfds, struct timeval *tv) {

    FD_ZERO(rfds);
    FD_SET(fd, rfds);

    int waiting = 0; 
    while (waiting > 0) {

        waiting = select(fd, rfds, NULL, NULL, tv);
        
        /* Handle when server connection is severed or error occurs*/
        if (!isThreadAlive)
            return false;
        else if (waiting == -1) 
            return false; 
    }

    return true;
}

int main() {
    /* Server init */
    char buffer[BUFF_SIZE];
    char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: heartbeat-M\r\n"
                  "Content-type: application/json\r\n\r\n"
                      "{\"is_alive\": true}\r\n";

    bool isActive = false; 
    int server_fd;

    /* Select init */
    struct timeeval; 
    fd_set rfds; 

    /* Thread init */
    pthread_t thread_id;
    struct timeval tv;

    tv.tv_sec = 1; 
    tv.tv_usec = 0;

    while (true) {
        
        while (!isActive) {
            isActive = check_connectivity(); 
            sleep(5);
        }

        int server_fd = start_server(); 
        if (server_fd <= -1) {
            perror("Server failure: ");
            continue; 
        } 


        struct sockaddr_in server_addr; 
        int server_addrlen = sizeof(server_addr);

        server_addr.sin_family = AF_INET; 
        server_addr.sin_addr.s_addr = INADDR_ANY; 
        server_addr.sin_port = htons(PORT); 

        struct sockaddr_in client_addr; 
        int client_addrlen = sizeof(client_addr);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
            perror("Webserver (bind)");
            return 1;
        }

        if (listen(server_fd, SOMAXCONN) != 0) {
            perror("Webserver (listen)");
            return 1; 
        }

        /* Print out the IP address and Port */
        if (server_addr.sin_addr.s_addr == 0) {
            printf("Server started at: http://%u.0.0.0:%d/\n", 
                server_addr.sin_addr.s_addr, PORT);
            fflush(stdout);
        } else {
            printf("Server started at: http://%u:%d/\n", 
                server_addr.sin_addr.s_addr, PORT);
            fflush(stdout);
        }

        if (pthread_create(&thread_id, NULL, check_connection_thread, NULL) != 0) {
            perror("pthread_create() error");
            exit(1);
        }
    
        isThreadAlive = true;

        fd_set rfds; 

        for (;;) {

            /* Waiting for the server to get a read or check if disconnection occurs */
            bool server_alive = wait_for_update(server_fd, &rfds, &tv);
            if (server_alive == false) {
                isActive = false;
                printf("Server disconnected or issue with select()");
                break;
            }
            printf("Active");

            int active_sock_fd = accept(server_fd, 
                (struct sockaddr *)&server_addr, (socklen_t *)&server_addrlen);

            if (active_sock_fd < 0) {
                perror("Webserver (accept)");
                continue;
            }

            printf("Connection Accepted\n");

            int sockn = getsockname(server_fd, 
                (struct sockaddr *)&client_addr, 
                (socklen_t *)&client_addrlen);
            
            if (sockn < 0) {
                perror("Unable to get the socket address");
                continue;
            }

            int read_sock = read(active_sock_fd, buffer, BUFF_SIZE);
            if (read_sock < 0) {
                perror("Unable to read socket");
                continue;
            }

            char method[BUFF_SIZE], uri[BUFF_SIZE], version[BUFF_SIZE]; 
            sscanf(buffer, "%s %s %s", method, uri, version);
            printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), 
                ntohs(client_addr.sin_port), method, version, uri);

            int write_sock = write(active_sock_fd, resp, strlen(resp)); 
            if (write_sock < 0){
                perror("Unable to write to socket");
                continue;
            } 


            close(active_sock_fd);
        }

        isActive = false;
        close(server_fd);
        pthread_cancel(thread_id);
    }

    close(server_fd);
    pthread_cancel(thread_id);

    return 0; 
}

