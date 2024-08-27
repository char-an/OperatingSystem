#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <signal.h>
#include <sys/types.h>

using namespace std;

void handler(int signo, siginfo_t *info, void *context)
{
    if (signo == SIGTERM)
    {
        cout << "[" << getpid() << "]" << " received SIGTERM\n";
        exit(0);
    }
}

void searchPattern(char *fileName, char *pattern, int start, int end) {
    pid_t pid = fork();
    if (pid < 0) {
        cerr << "Failed to fork." << endl;
        exit(1);
    } else if (pid == 0) {
        // Child process code
        //printf("Child Process: PID = %d, PGID = %d\n", getpid(), getpgrp());
        char *start_as_Str = new char[to_string(start).length() + 1];
        strcpy(start_as_Str, to_string(start).c_str());

        char *end_as_Str = new char[to_string(end).length() + 1];
        strcpy(end_as_Str, to_string(end).c_str());

        char *argv[] = { (char*)"./part3_searcher.out", fileName, pattern, start_as_Str, end_as_Str, nullptr };

        execv(argv[0], argv);

        cerr << "Failed to execv." << endl;
        delete[] start_as_Str;
        delete[] end_as_Str;
        exit(1);
    } else {
        // Parent process code
        //printf("Parent Process: PID = %d, PGID = %d\n", getpid(), getpgrp());
        cout << "[" << getpid() << "]" << " forked searcher child " << pid << endl;

        int status;
        waitpid(pid, &status, 0);
    }
    cout << "[" << getpid() << "]" << " searcher child returned " << endl;
}

void partitionSearch(char *fileName, char *pattern, int start, int end, int maxChunkSize) {
    cout << "[" << getpid() << "]" << " start position = " << start << " ; end position = " << end << endl;

    if (end - start + 1 > maxChunkSize) {
        int mid = (start + end) / 2;

        pid_t leftChild = fork();
        if (leftChild == 0) {
            //printf("LeftChild Process: PID = %d, PGID = %d\n", getpid(), getpgrp());
            partitionSearch(fileName, pattern, start, mid, maxChunkSize);
            exit(0);
        } else {
            cout << "[" << getpid() << "]" << " forked left child " << leftChild << endl;
            pid_t rightChild = fork();
            if (rightChild == 0) {
                //printf("RightChild Process: PID = %d, PGID = %d\n", getpid(), getpgrp());
                partitionSearch(fileName, pattern, mid + 1, end, maxChunkSize);
                exit(0);
            } else {
                //printf("Parent Process: PID = %d, PGID = %d\n", getpid(), getpgrp());
                cout << "[" << getpid() << "]" << " forked right child " << rightChild << endl;
                int status;
                waitpid(leftChild, &status, 0);
                waitpid(rightChild, &status, 0);
            }
        }
    } else {
        searchPattern(fileName, pattern, start, end);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        cout << "usage: ./partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position> <max-chunk-size>\nprovided arguments:\n";
        for (int i = 0; i < argc; i++)
            cout << argv[i] << "\n";
        return -1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = &handler;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    char *file_to_search_in = new char[strlen(argv[1]) + 1];
    strcpy(file_to_search_in, argv[1]);

    char *pattern_to_search_for = new char[strlen(argv[2]) + 1];
    strcpy(pattern_to_search_for, argv[2]);

    int search_start_position = atoi(argv[3]);
    int search_end_position = atoi(argv[4]);
    int max_chunk_size = atoi(argv[5]);

    partitionSearch(file_to_search_in, pattern_to_search_for, search_start_position, search_end_position, max_chunk_size);

    delete[] file_to_search_in;
    delete[] pattern_to_search_for;

    return 0;
}
