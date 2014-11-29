#include "Node.h"

#include <iostream>
#include <string>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>



using namespace std;

struct threadArgs {
    unsigned char nodeNum;
};

unsigned short portStart = 10050;
unsigned int numNodes;
string servAddr = "localhost";

volatile int numThreadsRunning = 0;
pthread_mutex_t numThreadsRunning_mutex = PTHREAD_MUTEX_INITIALIZER;

void *nodeThread(void *args);

int main(int argc, char **argv)
{
    numNodes = (unsigned int) stoi(argv[1]);
    string pyExec = "./Project_2_GenerateInput.py " + to_string(numNodes);
    system(pyExec.c_str()); // Gotta generate the input for our nodes!

    pthread_t *threadIDs = (pthread_t *) calloc(numNodes, sizeof(pthread_t));

    for(unsigned int i=0; i<numNodes; i++) {
        struct threadArgs *thisThreadArgs = (struct threadArgs *) calloc(1, sizeof(struct threadArgs));

        thisThreadArgs->nodeNum = i+1; // 1 is the monitor node, "left" is down, "right" is up

        // Let's spawn a thread!
        if(pthread_create(&(threadIDs[i]), NULL, nodeThread, (void *) thisThreadArgs) != 0) {
            cerr << "Unable to create thread for node " << i+1 << endl;
            exit(1);
        }
        pthread_mutex_lock(&numThreadsRunning_mutex); // Only modify with one thing at once
        numThreadsRunning++;
        pthread_mutex_unlock(&numThreadsRunning_mutex);
    }

    while(numThreadsRunning > 0)
        sleep(1); // Run until all threads done
}

void *nodeThread(void *args)
{
    // Grab the numbers from the arguments
    unsigned char nodeNum = ((struct threadArgs *) args)->nodeNum;
    free(args); // We allocated it
    args = NULL;

    Node thisNode(nodeNum, numNodes, servAddr, portStart); // Initialize, etc
    thisNode.run(); // Returns when the ring is finished

    // DONE
    pthread_mutex_lock(&numThreadsRunning_mutex); // Only modify with one thing at once
    numThreadsRunning--;
    pthread_mutex_unlock(&numThreadsRunning_mutex);
    return NULL;
}
