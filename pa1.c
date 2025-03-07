/*
# Copyright 2025 University of Kentucky
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
*/

/*
Please specify the group members here

# Student #1: Justin Tussey
# Student #2: Naleah Seabright
# Student #3:

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_EVENTS 64
#define MESSAGE_SIZE 16
#define DEFAULT_CLIENT_THREADS 4

char *server_ip = "127.0.0.1";
int server_port = 12345;
int num_client_threads = DEFAULT_CLIENT_THREADS;
int num_requests = 1000000;

/*
 * This structure is used to store per-thread data in the client
 */
typedef struct {
    int epoll_fd;        /* File descriptor for the epoll instance, used for monitoring events on the socket. */
    int socket_fd;       /* File descriptor for the client socket connected to the server. */
    long long total_rtt; /* Accumulated Round-Trip Time (RTT) for all messages sent and received (in microseconds). */
    long total_messages; /* Total number of messages sent and received. */
    float request_rate;  /* Computed request rate (requests per second) based on RTT and total messages. */
} client_thread_data_t;

/*
 * This function runs in a separate client thread to handle communication with the server
 */
void *client_thread_func(void *arg) {
    client_thread_data_t *data = (client_thread_data_t *)arg;
    struct epoll_event event, events[MAX_EVENTS];
    char send_buf[MESSAGE_SIZE] = "ABCDEFGHIJKMLNOP"; /* Send 16-Bytes message every time */
    char recv_buf[MESSAGE_SIZE];
    struct timeval start, end;
    long long rtt;
    int num_ready, i;
    struct sockaddr_in serverAddr;

    // Hint 1: register the "connected" client_thread's socket in the its epoll instance
    // Hint 2: use gettimeofday() and "struct timeval start, end" to record timestamp, which can be used to calculated RTT.

    /* TODO:
     * It sends messages to the server, waits for a response using epoll,
     * and measures the round-trip time (RTT) of this request-response.
     */

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    if(inet_pton(AF_INET, server_ip, &serverAddr.sin_addr) <= 0)
    {
        perror("Invalid server IP");
        close(data->socket_fd);
        return NULL;
    }
    data->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(data->socket_fd < 0)
    {
        perror("Created Socket Failed");
        return NULL;
    }
    if(connect(data->socket_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection failed");
        close(data->socket_fd);
        return NULL;
    }

    /* TODO:
     * The function exits after sending and receiving a predefined number of messages (num_requests).
     * It calculates the request rate based on total messages and RTT
     */

    data->epoll_fd = epoll_create(1);
    if(data->epoll_fd < 0)
    {
        perror("Created Epoll Failed");
        close(data->socket_fd);
        return NULL;
    }
    event.events = EPOLLIN;
    event.data.fd = data->socket_fd;
    if(epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->socket_fd, &event) < 0)
    {
        perror("Epoll control failed");
        close(data->socket_fd);
        close(data->epoll_fd);
        return NULL;
    }
    data->total_rtt = 0;
    data->total_messages = 0;
    for(int message_count = 0; message_count < num_requests; message_count++)
    {
        gettimeofday(&start, NULL);
        if(send(data->socket_fd, send_buf, MESSAGE_SIZE, 0) < 0)
        {
            perror("Send failed");
            break;
        }
        num_ready = epoll_wait(data->epoll_fd, events, MAX_EVENTS, 1000/*timeout*/);
        if(num_ready < 0)
        {
            perror("Epoll wait failed");
            break;
        }
        for(i = 0; i < num_ready; i++)
        {
            if(events[i].events & EPOLLIN)
            {
                if(recv(data->socket_fd, recv_buf, MESSAGE_SIZE, 0) <= 0)
                {
                    perror("Receive failed");
                    break;
                }
                gettimeofday(&end, NULL);
                rtt = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
                data->total_rtt += rtt;
                data->total_messages++;
                printf("RTT: %lld us\n", rtt);
            }
        }
    }

    if(data->total_messages > 0)
    {
        data->request_rate = (float)data->total_messages / (data->total_rtt / 1000000.0);
    }
    else
    {
        data->request_rate = 0;
    }

    printf("Client thread finished. Avg RTT: %lld us, Request rate: %.2f req/s\n", data->total_messages ? data->total_rtt / data->total_messages : 0, data->request_rate);
    close(data->socket_fd);
    close(data->epoll_fd);

    return NULL;
}

/*
 * This function orchestrates multiple client threads to send requests to a server,
 * collect performance data of each threads, and compute aggregated metrics of all threads.
 */
void run_client() {
    pthread_t threads[num_client_threads];
    client_thread_data_t thread_data[num_client_threads];
    struct sockaddr_in server_addr;

    /* TODO:
     * Create sockets and epoll instances for client threads
     * and connect these sockets of client threads to the server
     */

    for(int i = 0; i < num_client_threads; i++)
    {
        thread_data[i].socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(thread_data[i].socket_fd < 0)
        {
            perror("Created Socket Failed");
            exit(EXIT_FAILURE);
        }
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        {
            perror("Invalid server IP");
            exit(EXIT_FAILURE);
        }
        printf("Client %d connected to server\n", i);
        thread_data[i].epoll_fd = epoll_create(1);
    }

    // Hint: use thread_data to save the created socket and epoll instance for each thread
    // You will pass the thread_data to pthread_create() as below
    for (int i = 0; i < num_client_threads; i++) 
    {
        pthread_create(&threads[i], NULL, client_thread_func, &thread_data[i]);
    }

    /* TODO:
     * Wait for client threads to complete and aggregate metrics of all client threads
     */

     for(int i = 0; i < num_client_threads; i++)
     {
        pthread_join(threads[i], NULL);
     }

     long long total_rtt = 0;
     long long total_messages = 0;
     float total_request_rate = 0;
     for(int i; i < num_client_threads; i++)
     {
        total_rtt += thread_data[i].total_rtt;
        total_messages += thread_data[i].total_messages;
        total_request_rate += thread_data[i].request_rate;
     }
    if(total_messages > 0)
    {
        printf("Average RTT: %lld us\n", total_rtt / total_messages);
    }
    else
    {
        printf("Average RTT: No messages sent");
    }
    printf("Total Request Rate: %f messages/s\n", total_request_rate);
}

void run_server() {

    /* TODO:
     * Server creates listening socket and epoll instance.
     * Server registers the listening socket to epoll
     */
 

    /* Server's run-to-completion event loop */
    while (1) {
        /* TODO:
         * Server uses epoll to handle connection establishment with clients
         * or receive the message from clients and echo the message back
         */

    }

}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "server") == 0) {
        if (argc > 2) server_ip = argv[2];
        if (argc > 3) server_port = atoi(argv[3]);

        run_server();
    } else if (argc > 1 && strcmp(argv[1], "client") == 0) {
        if (argc > 2) server_ip = argv[2];
        if (argc > 3) server_port = atoi(argv[3]);
        if (argc > 4) num_client_threads = atoi(argv[4]);
        if (argc > 5) num_requests = atoi(argv[5]);

        run_client();
    } else {
        printf("Usage: %s <server|client> [server_ip server_port num_client_threads num_requests]\n", argv[0]);
    }

    return 0;
}
