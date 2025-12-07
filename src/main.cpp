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

    // Set defaults
    config->initial_value = 0;
    config->enable_flash = false;
    config->trace_lines = 1000; // Default trace size

    string line;
    while (getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse key = value format
        istringstream iss(line);
        string key, equals, value_str;

        if (iss >> key >> equals && equals == "=") {
            // Read rest of line as value (handles bool and numeric types)
            iss >> value_str;

            // DRAM Configuration
            if (key == "dram_banks") {
                config->dram_banks = stoul(value_str);
                config->bank_count = config->dram_banks;
            } else if (key == "dram_columns") {
                config->dram_columns = stoul(value_str);
            } else if (key == "dram_rows") {
                config->dram_rows = stoul(value_str);
            } else if (key == "row_buffer_size") {
                config->bank_size = stoul(value_str);
            }
            // Flash/Hybrid Configuration
            else if (key == "enable_flash") {
                config->enable_flash = (value_str == "true" || value_str == "1");
            } else if (key == "flash_capacity") {
                config->flash_capacity = stoull(value_str);
            } else if (key == "flash_page_size") {
                config->flash_page_size = stoul(value_str);
            }
            // Timing Parameters
            else if (key == "row_access_time") {
                config->row_access_time = stoul(value_str);
            } else if (key == "column_access_time") {
                config->column_access_time = stoul(value_str);
            } else if (key == "refresh_interval") {
                config->refresh_interval = stoul(value_str);
            }
            // Simulation Parameters
            else if (key == "trace_lines") {
                config->trace_lines = stoull(value_str);
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
