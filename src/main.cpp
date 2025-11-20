#include <iostream>
#include <string>
#include <fstream>
#include "./../include/memory_simulator.h"

using namespace std;

typedef struct config{

    int bank_count;
    int bank_size;
    int initial_value;

} Config;

int read_config(string file_path, Config * config){

    ifstream file(file_path);
        if( !(file.is_open()) ){
            cerr << "ERROR, INVALID CONFIG FILE PATH!!"<< endl;
            return 1;
        }
    cout << "Reading config file" << endl;
    string label;

    file >> label >> label >> config->bank_count;
    file >> label >> label >> config->bank_size;
    file >> label >> label >> config->initial_value;

    file.close();
    return 0;

}




int main(int argc, char* argv[]) {
    cout << "HybridMemSim - Hybrid Memory Simulator" << endl;

    //parse command line arguments
        if (argc != 2){
            cout << "Argument Error" << endl;
            //cout << "Usage: <number of banks > <bank size in words> <path to config file>" << endl;
            cout << "Usage: <path to config file>" << endl;
            return 1;
        }
        //build dram array
        // int bank_count = stoi(argv[1]);
        // int bank_size = stoi(argv[2]);
        //string config_path = argv[3]
        string config_path = argv[1];
        Config config;
    
    if (read_config(config_path,&config)) return 1;

    MemorySimulator memorysim;


    
    // TODO: Parse command line arguments
    // TODO: Load configuration
    // TODO: Initialize simulator
    // TODO: Run simulation
    // TODO: Output results

    return 0;
}
