#include <algorithm>
#include <math.h>
#include "pagetable.h"

PageTable::PageTable(int page_size, uint32_t memory_size) {
    _page_size = page_size;
    _memory_size = memory_size;
    _free_frames = memory_size / page_size;
}

PageTable::~PageTable() {
}

std::vector<std::string> PageTable::sortedKeys() {
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++) {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

int PageTable::getPageSize() {
    return _page_size;
}

int PageTable::getFreeFrames() {
    return _free_frames;
}

std::string PageTable::getLookupString(uint32_t pid, int page_number) {
    // Combination of pid and page number act as the key to look up frame number
    return std::to_string(pid) + "|" + std::to_string(page_number);
}

bool PageTable::doesPidOwnPage(uint32_t pid, int page_number) {
    return (_table.count(getLookupString(pid, page_number)) > 0);
}

void PageTable::addEntry(uint32_t pid, int page_number) {
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = getLookupString(pid, page_number);

    // Find free frame
    // TODO: implement this!
    int frame;
    bool frame_found;
    for (frame = 0; frame < _memory_size / _page_size; frame++) {
        frame_found = true;
        for (std::map<std::string, int>::iterator it = _table.begin(); it != _table.end(); it++) {
            // Disqualify candidate frame if it is already in the table
            if (it->second == frame) {
                frame_found = false;
                break;
            }
        }
        if (frame_found) break;
    }
    if (!frame_found) {
        std::cout << "error: illegal frame allocation (should have been detected earlier)" << std::endl;
        return;
    }
    else {      
        _free_frames--;
        _table[entry] = frame;
    }
}

int PageTable::getPage(uint32_t virtual_address) {
    return virtual_address / _page_size;
}

int PageTable::getOffset(uint32_t virtual_address) {
    return virtual_address % _page_size;
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address) {
    // Convert virtual address to page_number and page_offset
    // TODO: implement this!
    int page_number = getPage(virtual_address);
    int page_offset = getOffset(virtual_address);

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = getLookupString(pid, page_number);

    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0) {
        // TODO: implement this!
        int frame_number = _table[entry];
        address = frame_number * _page_size + page_offset;
    }

    return address;
}

void PageTable::print() {
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++) {
        // TODO: print all pages
    }
}
