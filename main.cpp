#include <iostream>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>

struct Node
{
    int value;
    Node *next;
    std::mutex node_mutex;

    Node(int _value) : value(_value), next(nullptr) {}
};

class FineGrainedQueue
{
private:
    Node *head;
    std::mutex queue_mutex;

public:
    FineGrainedQueue() : head(nullptr) {}

    void insertIntoMiddle(int value, int pos)
    {
        if (pos < 0)
            return;

        std::random_device rd;                            // Only used once to initialise (seed) engine
        std::mt19937 rng(rd());                           // Random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uni(0, 3); // Guaranteed unbiased

        std::this_thread::sleep_for(std::chrono::seconds(uni(rng)));

        Node *newNode = new Node(value);

        if (!head && pos >= 0)
        {
            queue_mutex.lock();
            head = newNode;
            queue_mutex.unlock();
            return;
        }

        queue_mutex.lock();
        Node *current = head;
        queue_mutex.unlock();

        int queueLength = 1;

        while (current->next != nullptr) // just to detect queue length
        {
            current = current->next;
            ++queueLength;
        }

        if (pos >= queueLength)
        {
            current->node_mutex.lock();
            current->next = newNode; // we have last element of the queue from previous loop
            current->node_mutex.unlock();
            return;
        }

        else
        {
            current = head;
            int currentIndex = 0;
            Node *prev;

            while (current->next != nullptr && currentIndex < pos - 1)
            {
                prev = current;
                current = current->next;
                currentIndex++;
            }

            std::lock_guard<std::mutex> nodeLock(current->node_mutex);
            newNode->next = current->next;
            current->next = newNode;
        }
    }

    void printQueue()
    {
        Node *current = head;
        while (current)
        {
            std::cout << current->value << "\t";
            current = current->next;
        }
        std::cout << std::endl;
    }
};

int main()
{
    bool multiThread = true;
    FineGrainedQueue queue;

    if (multiThread)
    {
        std::thread t1(&FineGrainedQueue::insertIntoMiddle, &queue, 7, 0);
        std::thread t2(&FineGrainedQueue::insertIntoMiddle, &queue, 11, 1);
        std::thread t3(&FineGrainedQueue::insertIntoMiddle, &queue, 15, 2);
        std::thread t4(&FineGrainedQueue::insertIntoMiddle, &queue, 2, 1);
        std::thread t5(&FineGrainedQueue::insertIntoMiddle, &queue, 3, 777);
        std::thread t6(&FineGrainedQueue::insertIntoMiddle, &queue, 65, -1000);
        std::thread t7(&FineGrainedQueue::insertIntoMiddle, &queue, 8, 3);

        t1.detach();
        t2.detach();
        t3.detach();
        t4.detach();
        t5.detach();
        t6.detach();
        t7.detach();

        for (int i = 0; i < 500; ++i)
        {
            srand(time(NULL));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            queue.printQueue();
        }
    }
}