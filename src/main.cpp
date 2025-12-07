#include "./../include/memory_simulator.h"

using namespace std;


int read_config(string file_path, Config * config){
    // Use default config path if empty
    if (file_path.empty()) {
        file_path = "./config/default.cfg";
    }

    ifstream file(file_path);
    if (!file.is_open()) {
        cerr << "ERROR: Could not open config file: " << file_path << endl;
        return 1;
    }

    cout << "Reading config file: " << file_path << endl;

    string line;
    while (getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse key = value format
        istringstream iss(line);
        string key, equals;
        int value;

        if (iss >> key >> equals >> value && equals == "=") {
            if (key == "dram_banks") {
                config->bank_count = value;
            } else if (key == "row_buffer_size" || key == "dram_columns") {
                config->bank_size = value;
            } else if (key == "dram_rows") {
                // Store initial_value or map as needed
                config->initial_value = 0; // Default value
            }
        }
    }

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

    // Parse command line arguments
    string config_path;
    if (argc >= 2) {
        // Use provided config file path
        config_path = argv[1];
    } else {
        // Use default config file
        config_path = "";  // Empty string triggers default in read_config
    }

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
                memorysim.printStats();
                cout << "\nPress Enter to continue...";
                cin.get();
                display_menu();
                break;

            case 'c':
            case 'C':
                cout << "\n=== Clearing Memory ===" << endl;
                //clear method here
                memorysim.clear();
                cout << "Memory cleared successfully!" << endl;
                cout << "\nPress Enter to continue...";
                cin.get();
                display_menu();
                break;

            case 'r':
            case 'R':
                cout << "\n=== Running Memory Simulator ===" << endl;
                //run method here
                memorysim.run();
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
