#include<iostream>
#include<fstream>
using namespace std;

int main(int argc, char **argv){
    char *process_info_file = argv[1];
    ifstream file(process_info_file);
    string burst_info;
    while(getline(file, burst_info)){
        if(!burst_info.empty() && burst_info[0] != '<'){
            cout << burst_info << endl;
        }
    }
    return 0;
}