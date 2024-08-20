#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> 

using namespace std;

int printOccurrences(const string& file_to_search_in, const string& pattern_to_search_for, int start_position, int end_position)
{
    ifstream file(file_to_search_in);
    file.seekg(start_position);
    
    int length= end_position -  start_position+1;
    string txt(length, '\0');
    file.read(&txt[0],length);
    file.close();
    int f=1;
    size_t found=txt.find(pattern_to_search_for);
    while(found != string::npos){
    	cout << "found at" << found <<endl;
    	f=0;
    	found=txt.find(pattern_to_search_for,found+1);    
    }
    
    cout << "found "<<found<<"\n";
    if(f==1){
    	cout << "not found\n";
    }
    return f;
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cout << "Usage: ./partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position>\n";
        for (int i = 0; i < argc; i++)
            cout << argv[i] << "\n";
        return -1;
    }
    
    string file_to_search_in = argv[1];
    string pattern_to_search_for = argv[2];
    int search_start_position = atoi(argv[3]);
    int search_end_position = atoi(argv[4]);    
     
    int f=printOccurrences(file_to_search_in, pattern_to_search_for, search_start_position, search_end_position);
    if(f==0){return 1;}
    else if(f==1){return 0;}
    return 2;
}

