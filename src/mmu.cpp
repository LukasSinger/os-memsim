#include <iostream>
#include <algorithm>
#include "mmu.h"

Mmu::Mmu(int memory_size) {
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu() {
}

uint32_t Mmu::createProcess() {
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;

    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address) {
    int i;
    Process *proc = getProcess(pid);

    if (proc != NULL) {
        Variable *var = new Variable();
        var->name = var_name;
        var->type = type;
        var->virtual_address = address;
        var->size = size;
        proc->variables.push_back(var);
    }
}

Process *Mmu::getProcess(uint32_t pid) {
    std::vector<Process *>::iterator it = std::find_if(_processes.begin(), _processes.end(), [pid](Process *p) {
        return p != nullptr && p->pid == pid;
        });
    if (it != _processes.end()) {
        return *it;
    } else {
        return NULL;
    }
}

std::vector<Variable *> Mmu::sortedAllocations(Process *process) {
    std::vector<Variable *> variables;

    if (process->variables.size() > 1) {
        std::vector<Variable *>::iterator it;
        for (it = process->variables.begin(); it != process->variables.end(); it++) {
            if ((*it)->type == DataType::FreeSpace) continue;
            variables.push_back(*it);
        }

        std::sort(variables.begin(), variables.end(), VariableAllocationComparator());
    }

    return variables;
}

Variable* Mmu::getVariable(Process* process, const std::string& name) {
    if (process == nullptr) {
        return nullptr;
    }

    for (Variable* v : process->variables) {
        if (v->name == name && v->type != DataType::FreeSpace) {
            return v;
        }
    }
    return nullptr;
}

void Mmu::printProcesses() {
    for (Process* p : _processes) {
        if (p != nullptr) {
            std::cout << p->pid << std::endl;
        }
    }
}

void Mmu::print() {
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++) {
        for (j = 0; j < _processes[i]->variables.size(); j++) {
            // TODO: print all variables (excluding those of type DataType::FreeSpace)
            Variable* var = _processes[i]->variables[j];

            if (var->type == DataType::FreeSpace) {
                continue;
            }

            printf("%5d | %-13s |   0x%08X | %10d\n", _processes[i]->pid, var->name.c_str(), var->virtual_address, var->size);
        }
    }
}

void Mmu::removeVariable(uint32_t pid, const std::string& var_name) {
    Process* process = getProcess(pid);
    if (process == nullptr) {
        return;
    }

    for (std::vector<Variable*>::iterator it = process->variables.begin(); it != process->variables.end(); ) {
        Variable* var = *it;

        if (var->name == var_name && var->type != DataType::FreeSpace) {
            delete var;
            it = process->variables.erase(it);
        } else {
            ++it;
        }
    }
}

void Mmu::removeProcess(uint32_t pid) {
    // Loop through all processes until we find the one we are looking for
    for (std::vector<Process*>::iterator it = _processes.begin(); it != _processes.end(); ) {
        // Get the current process
        Process* process = *it;

        // Check if this is the process we are looking for
        if ((process != nullptr) && (process->pid == pid)) {
            // Delete all variables in that process
            for (std::vector<Variable*>::iterator vit = process->variables.begin(); vit != process->variables.end(); ++vit) {
                delete *vit;
            }

            // Delete the process itself
            delete process;

            // Remove it from the vector
            it = _processes.erase(it);

            return;
        } 
        else {
            ++it;
        }
    }
}