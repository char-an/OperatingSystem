#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <vector>
#include <cstring>

using namespace std;

void searchPattern(char *fileName, char *pattern, int start, int end) {
    pid_t pid = fork();
    if (pid < 0) {
        cerr << "Failed to fork." << endl;
        exit(1);
    } else if (pid == 0) {
        char *start_as_Str = new char[to_string(start).length() + 1];
        strcpy(start_as_Str, to_string(start).c_str());

        char *end_as_Str = new char[to_string(end).length() + 1];
        strcpy(end_as_Str, to_string(end).c_str());

        char *argv[] = { (char*)"./part2_searcher.out", fileName, pattern, start_as_Str, end_as_Str, nullptr };

        execv(argv[0], argv);

        cerr << "Failed to execv." << endl;
        delete[] start_as_Str;
        delete[] end_as_Str;
        exit(1);
    } else {
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
            partitionSearch(fileName, pattern, start, mid, maxChunkSize);
            cout << "[" << getpid() << "]" << " left child returned " << endl;
            exit(0);
        } else {
            cout << "[" << getpid() << "]" << " forked left child " << leftChild << endl;

            pid_t rightChild = fork();
            if (rightChild == 0) {
                partitionSearch(fileName, pattern, mid + 1, end, maxChunkSize);
                cout << "[" << getpid() << "]" << " right child returned " << endl;
                exit(0);
            } else {
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
    if(argc != 6)
	{
		cout <<"usage: ./partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position> <max-chunk-size>\nprovided arguments:\n";
		for(int i = 0; i < argc; i++)
			cout << argv[i] << "\n";
		return -1;
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
