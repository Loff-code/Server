#include "funcs.h"

request_t request_queue[QUEUE_SIZE];
int queue_size = 0;

void swap(request_t *a, request_t *b)
{
    request_t temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(int index)
{
    int parent = (index - 1) / 2;
    while (index > 0 && request_queue[index].prio > request_queue[parent].prio)
    {
        swap(&request_queue[index], &request_queue[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapify_down(int index)
{
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < queue_size && request_queue[left].prio > request_queue[largest].prio)
    {
        largest = left;
    }
    if (right < queue_size && request_queue[right].prio > request_queue[largest].prio)
    {
        largest = right;
    }
    if (largest != index)
    {
        swap(&request_queue[index], &request_queue[largest]);
        heapify_down(largest);
    }
}

void enqueue(request_t request)
{
    pthread_mutex_lock(&queue_mutex);

    if (queue_size < QUEUE_SIZE)
    {
        request_queue[queue_size] = request;
        heapify_up(queue_size);
        queue_size++;
    }

    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

request_t dequeue()
{
    pthread_mutex_lock(&queue_mutex);

    while (queue_size == 0)
    {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    request_t highest_prio_request = request_queue[0];
    request_queue[0] = request_queue[queue_size - 1];
    queue_size--;

    heapify_down(0);

    pthread_mutex_unlock(&queue_mutex);
    return highest_prio_request;
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
            write(request.client_socket, &key, sizeof(key));
            close(request.client_socket);
        }
        else
        {
            enqueue(request);
        }
    }

    return 0;
}
