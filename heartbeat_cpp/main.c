#include <stdio.h> 
#include <stdbool.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <errno.h>

#include <netdb.h> 
#include <netinet/in.h> 
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <pthread.h>


#define PORT 8000
#define BUFF_SIZE 2024 

bool isThreadAlive = false;

/* Computing the checksum https://www.cs.dartmouth.edu/~sergey/cs60/lab3/icmp4-rawsend.c */
uint16_t
checksum (char *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(unsigned char*) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

bool check_connectivity() {
    char buffer[64]; 
    memset(&buffer, 0, 64); 

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
    icmp->icmp_hun.ih_idseq.icd_seq = 1;
    icmp->icmp_hun.ih_idseq.icd_id = 0;

    icmp->icmp_cksum = checksum(buffer, 64);

    struct sockaddr_in dest_addr; 
    dest_addr.sin_family = AF_INET; 
    inet_aton("8.8.8.8", &dest_addr.sin_addr); 
    dest_addr.sin_port = htons(443);
    int dest_addr_len = sizeof(dest_addr);

    struct sockaddr_in from;
    int from_len = sizeof(from);

    printf("Type: %d\n", icmp->icmp_type);
    printf("Code: %d\n\n", icmp->icmp_code);

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

    pthread_exit(NULL);
}

int start_server() {
    int server_fd; 

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE);
    }

    return server_fd; 
}

int main() {

    pthread_t thread_id;
    bool isActive = false; 
    int server_fd;

    while (true) {
        
        while (!isActive) {
            isActive = check_connectivity(); 
            sleep(5);
        }

        char buffer[BUFF_SIZE];
        char resp[] = "HTTP/1.0 200 OK\r\n"
                      "Server: heartbeat-M\r\n"
                      "Content-type: application/json\r\n\r\n"
                      "{\"is_alive\": true}\r\n";

        int server_fd = start_server(); 

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

        for (;;) {
            int active_sock_fd = accept(server_fd, 
                (struct sockaddr *)&server_addr, (socklen_t *)&server_addrlen);

            printf("Checking thread: %d\n", isThreadAlive);
            if (isThreadAlive == false) 
                break; 

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

        printf("Break\n"); 
        close(server_fd);
        pthread_cancel(thread_id);
    }

    close(server_fd);
    pthread_cancel(thread_id);

    return 0; 
}
