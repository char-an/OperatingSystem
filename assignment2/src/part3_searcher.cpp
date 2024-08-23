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

	//TODO
	int length_pattern = strlen(pattern_to_search_for);
	// cout << "length of pattern :" << length_pattern << endl;

    //file open
	ifstream file(file_to_search_in); 

	string file_string;
    char c; 
	// loop getting single characters
    while (file.get(c)){
		file_string += c;
	}        
    
	// close file
    file.close();                



	for (int i = search_start_position; i <= search_end_position; i++)
    {
        if (file_string.substr(i, length_pattern) == pattern_to_search_for)
        {
            return 1;
        }
    }
	
	//cout << "[-1] didn't find\n";
	return 0;
}