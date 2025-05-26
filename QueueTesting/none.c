#include "funcs.h"

typedef struct node
{
    request_t request;
    struct node *next;
} node_t;

node_t *head = NULL;
node_t *tail = NULL;

void enqueue(request_t request)
{
    pthread_mutex_lock(&queue_mutex);
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->request = request;
    new_node->next = NULL;
    if (tail == NULL)
    {
        head = tail = new_node;
    }
    else
    {
        tail->next = new_node;
        tail = new_node;
    }
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

request_t dequeue()
{
    pthread_mutex_lock(&queue_mutex);
    while (head == NULL)
    {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }
    node_t *temp = head;
    request_t request = temp->request;
    head = head->next;
    if (head == NULL)
    {
        tail = NULL;
    }
    free(temp);
    pthread_mutex_unlock(&queue_mutex);
    return request;
}

int main(int argc, char *argv[])
{

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(5000);

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind failed");
        exit(1);
    }

    listen(server_fd, 100);

    struct sockaddr_in cli_addr;
    int cli_length = sizeof(cli_addr);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    while (1)
    {
        int client_socket = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_length);
        if (client_socket < 0)
        {
            perror("Accept failed");
            exit(1);
        }

        char buffer[PACKET_REQUEST_SIZE];
        bzero(buffer, PACKET_REQUEST_SIZE);
        read(client_socket, buffer, PACKET_REQUEST_SIZE);

        request_t request = pack(buffer, client_socket);
        uint64_t key = 0;
        if (checkCache(request.hash, &key))
        {
            key = htobe64(key);
            write(request.client_socket, &key, 8);

            close(request.client_socket);
        }
        else
        {
            enqueue(request);
        }
    }

    return 0;
}
