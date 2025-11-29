#include "./../include/memory_simulator.h"

using namespace std;


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

void display_menu() {
    cout << "\n\n***Welcome to the Hybrid Memory Simulator***\n\nPlease make a selection:\n" << endl;
    cout << "p\t-- print all memory statistics" << endl;
    cout << "c\t-- clear memory" << endl;
    cout << "r\t-- run the memory simulator" << endl;
    cout << "h\t-- display this help menu" << endl;
    cout << "q\t-- exit the simulator\n" << endl;
    cout << "------------------------------------------------------------------\n" << endl;
    cout << "Selection: ";
}





int main(int argc, char* argv[]) {
    cout << "HybridMemSim - Hybrid Memory Simulator" << endl;

    //parse command line arguments
        if (argc != 2){
            cout << "Argument Error" << endl;
            cout << "Usage: <path to config file>" << endl;
            return 1;
        }

        string config_path = argv[1];
        Config config;
    
    if (read_config(config_path, &config)) return 1;

    MemorySimulator memorysim(config);
    char selection;
    bool running = true;

     display_menu();
    
    while(running) {
        selection = cin.get();
        cin.ignore(1000, '\n');  // Clear input buffer
        
        switch (selection) {
            case 'p':
            case 'P':
                cout << "\n=== Memory Statistics ===" << endl;
                //print method here
                cout << "\nPress Enter to continue...";
                cin.get();
                display_menu();
                break;

            case 'c':
            case 'C':
                cout << "\n=== Clearing Memory ===" << endl;
                //clear method here
                cout << "Memory cleared successfully!" << endl;
                cout << "\nPress Enter to continue...";
                cin.get();
                display_menu();
                break;

            case 'r':
            case 'R':
                cout << "\n=== Running Memory Simulator ===" << endl;
                //run method here
                cout << "\nSimulation complete!" << endl;
                cout << "\nPress Enter to continue...";
                cin.get();
                display_menu();
                break;

            case 'h':
            case 'H':
                display_menu();
                break;

            case 'q':
            case 'Q':
                cout << "\n=== Exiting Simulator ===" << endl;
                running = false;
                break;

            case '\n':
            case '\r':
                // Ignore empty lines
                break;

            default:
                cout << "Invalid selection: '" << selection << "'" << endl;
                cout << "Please try again or press 'h' for help." << endl;
                cout << "Selection: ";
                break;
        }
    }

    
    // TODO: Parse command line arguments
    // TODO: Load configuration
    // TODO: Initialize simulator
    // TODO: Run simulation
    // TODO: Output results

    return 0;
}
