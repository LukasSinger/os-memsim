#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include "mmu.h"
#include "pagetable.h"

// 64 MB (64 * 1024 * 1024)
#define PHYSICAL_MEMORY 67108864

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table, bool cmd = false);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);
void splitString(std::string text, char d, std::vector<std::string> &result);

int main(int argc, char **argv) {
    // Ensure user specified page size as a command line parameter
    if (argc < 2) {
        std::cerr << "Error: you must specify the page size" << std::endl;
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory' (raw array of bytes)
    uint8_t *memory = new uint8_t[PHYSICAL_MEMORY];

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(PHYSICAL_MEMORY);
    PageTable *page_table = new PageTable(page_size, PHYSICAL_MEMORY);

    // Prompt loop
    std::string command = "";
    while (command != "exit") {
        // Get next command
        std::cout << "> ";
        std::getline(std::cin, command);

        // Handle command
        // TODO: implement this!
        std::vector<std::string> args;
        splitString(command, ' ', args);
        if (args.empty()) continue;
        command = args.at(0);
        if (command == "create") {
            int text_size = std::stoi(args.at(1));
            int data_size = std::stoi(args.at(2));
            createProcess(text_size, data_size, mmu, page_table);
        } else if (command == "allocate") {
            int pid = std::stoi(args.at(1));
            std::string var_name = args.at(2);
            std::string type_input = args.at(3);
            DataType type;
            //Short, Int, Float, Long, Double
            if (type_input == "char") type = Char;
            else if (type_input == "short") type = Short;
            else if (type_input == "int") type = Int;
            else if (type_input == "float") type = Float;
            else if (type_input == "long") type = Long;
            else if (type_input == "double") type = Double;
            else {
                std::cout << "error: type not recognized" << std::endl;
                continue;
            }
            int num_elements = std::stoi(args.at(4));
            allocateVariable(pid, var_name, type, num_elements, mmu, page_table, true);
        }
        else if (command == "set") {
            uint32_t pid = std::stoi(args.at(1));
            std::string var_name = args.at(2);
            uint32_t offset = std::stoi(args.at(3));

            // Access the variable 
            // Process not found error check
            Process *process = mmu->getProcess(pid);
            if (!process) {
                std::cout << "error: process not found" << std::endl;
                continue;
            }
            
            // Find variable    
            Variable *var = mmu->getVariable(process, var_name);
            
            // Variable error check
            if (var == nullptr) {
                std::cout << "error: variable not found" << std::endl;
                continue;
            }

            // Compute number of values being set
            uint32_t num_values = args.size() - 4;

            // Determine size of each element
            uint32_t elementSizeBytes = 1;
            if (var->type == Char) elementSizeBytes = 1;
            else if (var->type == Short) elementSizeBytes = 2;
            else if (var->type == Int || var->type == Float) elementSizeBytes = 4;
            else if (var->type == Long || var->type == Double) elementSizeBytes = 8;

            // Total number of elements in the variable
            uint32_t numElements = var->size / elementSizeBytes;

            // Index out of bounds error check
            if (offset + num_values > numElements) {
                std::cout << "error: index out of range" << std::endl;
                continue;
            }

            // Loop through all values provided
            for (uint32_t i = 4; i < args.size(); i++) {
                std::string val_str = args[i];
                uint32_t index = offset + (i - 4);

                // Use the appropriate data type to call setVariable
                if (var->type == Char) {
                    char val = val_str[0];
                    setVariable(pid, var_name, index, &val, mmu, page_table, memory);
                }
                else if (var->type == Short) {
                    short val = std::stoi(val_str);
                    setVariable(pid, var_name, index, &val, mmu, page_table, memory);
                }
                else if (var->type == Int) {
                    int val = std::stoi(val_str);
                    setVariable(pid, var_name, index, &val, mmu, page_table, memory);
                }
                else if (var->type == Float) {
                    float val = std::stof(val_str);
                    setVariable(pid, var_name, index, &val, mmu, page_table, memory);
                }
                else if (var->type == Long) {
                    long val = std::stol(val_str);
                    setVariable(pid, var_name, index, &val, mmu, page_table, memory);
                }
                else if (var->type == Double) {
                    double val = std::stod(val_str);
                    setVariable(pid, var_name, index, &val, mmu, page_table, memory);
                }
            }
        } 
        else if (command == "print") {
            std::string arg = args.at(1);

            if (arg == "mmu") {
                mmu->print();
            }
            else if (arg == "page") {
                page_table->print();
            }
            else if (arg == "processes") {
                // print all PIDs
                mmu->printProcesses();
            }
            else {
                // assume format PID:var_name
                
            }
        } 
        else if (command == "free") {
            
        } 
        else if (command == "terminate") {
            
        } 
        else if (command == "exit") {
            break;
        } else {
            std::cout << "error: command not recognized" << std::endl;
        }
    }

    // Clean up
    delete[] memory;
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size) {
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std::endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std::endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std::endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std::endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std::endl;
    std::cout << "  * print <object> (prints data)" << std::endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std::endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std::endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std::endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std::endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table) {
    // TODO: implement this!
    //   - create new process in the MMU
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    //   - print pid
    uint32_t pid = mmu->createProcess();
    allocateVariable(pid, "<TEXT>", Char, text_size, mmu, page_table);
    allocateVariable(pid, "<GLOBALS>", Char, data_size, mmu, page_table);
    allocateVariable(pid, "<STACK>", Char, 65536, mmu, page_table);
    printf("%d\n", pid);
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table, bool cmd) {
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address

    // Check whether PID exists
    if (mmu->getProcess(pid) == NULL) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    // Compute space required
    uint32_t size = num_elements;
    if (type == Short) size *= 2;
    else if (type == Int || type == Float) size *= 4;
    else if (type == Long || type == Double) size *= 8;

    // Find first fit
    bool addr_found = false;
    uint32_t check_virtual_address = 0;
    Process *process = mmu->getProcess(pid);
    std::vector<Variable *> variables = mmu->sortedAllocations(process);
    if (variables.size() > 0) {
        for (std::vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++) {
            if ((*it)->name == var_name) {
                std::cout << "error: variable already exists" << std::endl;
                return;
            }
            if (check_virtual_address + size <= (*it)->virtual_address) {
                addr_found = true;
                break;
            } else if (!addr_found) {
                check_virtual_address = (*it)->virtual_address + (*it)->size;
            }
        }
    }
    if (!addr_found) {
        // Ensure first page has been allocated
        if (!page_table->doesPidOwnPage(pid, 0)) {
            if (page_table->getFreeFrames() < 1 + size / page_table->getPageSize()) {
                std::cout << "error: not enough memory remaining" << std::endl;
                return;
            }
            page_table->addEntry(pid, 0);
        }
        // Check end of last page allocated
        int last_page = page_table->getPage(check_virtual_address);
        if (check_virtual_address + size >= (last_page + 1) * page_table->getPageSize()) {
            // Allocate new pages
            uint32_t allocate_end = check_virtual_address + size;
            int final_page_allocation = page_table->getPage(allocate_end);
            if (page_table->getFreeFrames() < final_page_allocation - last_page) {
                std::cout << "error: not enough memory remaining" << std::endl;
                return;
            }
            for (int i = last_page + 1; i <= final_page_allocation; i++) {
                page_table->addEntry(pid, i);
            }
        }
    }

    // Record variable
    mmu->addVariableToProcess(pid, var_name, type, size, check_virtual_address);

    // Print address if this was a command
    if (cmd) std::cout << check_virtual_address << std::endl;
}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory) {
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)

    // Access the variable (some redundant code with the checks, but better safe than sorry)
    // Process not found error check
    Process *process = mmu->getProcess(pid);
    if (process == nullptr) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    // Find variable
    Variable *var = mmu->getVariable(process, var_name);

    // Variable error check
    if (var == nullptr) {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    // Get the size of the data type
    uint32_t elementSizeBytes = 1;
    if (var->type == Char) elementSizeBytes = 1;
    else if (var->type == Short) elementSizeBytes = 2;
    else if (var->type == Int || var->type == Float) elementSizeBytes = 4;
    else if (var->type == Long || var->type == Double) elementSizeBytes = 8;

    // Compute virtual address
    uint32_t virtual_address = var->virtual_address + (offset * elementSizeBytes);

    // Translate to physical address
    int physical_address = page_table->getPhysicalAddress(pid, virtual_address);

    // Physical memory error
    if (physical_address < 0 || physical_address >= PHYSICAL_MEMORY) {
        std::cout << "error: invalid memory access at " << physical_address << std::endl;
        return;
    }

    // Write value into memory
    memcpy(&memory[physical_address], value, elementSizeBytes);
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table) {
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table) {
    // TODO: implement this!
    //   - remove process from MMU
    //   - free all pages associated with given process
}

/*
   From osshell
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string> &result) {
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++) {
        char c = text[i];
        switch (state) {
        case NONE:
            if (c != d) {
                if (c == '\"') {
                    state = IN_STRING;
                    token = "";
                } else {
                    state = IN_WORD;
                    token = c;
                }
            }
            break;
        case IN_WORD:
            if (c == d) {
                result.push_back(token);
                state = NONE;
            } else {
                token += c;
            }
            break;
        case IN_STRING:
            if (c == '\"') {
                result.push_back(token);
                state = NONE;
            } else {
                token += c;
            }
            break;
        }
    }
    if (state != NONE) {
        result.push_back(token);
    }
}