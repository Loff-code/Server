#include "funcs.h"

request_t request_queue[QUEUE_SIZE];
int queue_size = -1;

int peek()
{
    int highestPriority = -1;
    int highestIndex = -1;

    for (int i = 0; i <= queue_size; i++)
    {
        if (request_queue[i].prio > highestPriority)
        {
            highestPriority = request_queue[i].prio;
            highestIndex = i;
        }
    }
    return highestIndex;
}

void enqueue(request_t request)
{
    pthread_mutex_lock(&queue_mutex);
    if (queue_size < QUEUE_SIZE - 1)
    {
        queue_size++;
        request_queue[queue_size] = request;
    }
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

request_t dequeue()
{
    pthread_mutex_lock(&queue_mutex);

    while (queue_size == -1)
    {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    int highestIndex = peek();
    request_t highest_prio_request = request_queue[highestIndex];

    for (int i = highestIndex; i < queue_size; i++)
    {
        request_queue[i] = request_queue[i + 1];
    }
    queue_size--;

    pthread_mutex_unlock(&queue_mutex);
    return highest_prio_request;
}
void checkQueue(uint8_t *hash)
{
    pthread_mutex_lock(&queue_mutex);
    for (int i = 0; i < queue_size; i++)
    {
        if (memcmp(request_queue[i].hash, hash, 32) == 0)
        {
            request_queue[i].prio = 20;
            pthread_mutex_unlock(&queue_mutex);
            printf("Request put in front\n");
        }
    }
    pthread_mutex_unlock(&queue_mutex);
}

int main(int argc, char *argv[])
{

    return mainer();
}
