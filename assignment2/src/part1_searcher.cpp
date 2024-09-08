#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <signal.h>

using namespace std;

int main(int argc, char **argv)
{
	if(argc != 5)
	{
		cout <<"usage: ./partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position>\nprovided arguments:\n";
		for(int i = 0; i < argc; i++)
			cout << argv[i] << "\n";
		return -1;
	}
	
	char *file_to_search_in = argv[1];
	char *pattern_to_search_for = argv[2];
	int search_start_position = atoi(argv[3]);
	int search_end_position = atoi(argv[4]);

	int length_pattern = strlen(pattern_to_search_for);

	ifstream file(file_to_search_in); 

	string file_string;
    char c; 
    while (file.get(c)){
		file_string += c;
	}        
    
	// cout << file_string;
    file.close();                

	int f=0;
	for (int i=search_start_position; i<=search_end_position; i++)
    {
        if (file_string.substr(i, length_pattern) == pattern_to_search_for)
        {
            cout << "[" << getpid() << "]" << " found at "  << i << endl;
			f=1;
            return 1;
        }
    }
	
	if(f==0){
		cout << "[" << getpid() << "] " << "didn't find\n";
		return 0;
	}
}