#include "example_config.h"
#include "util/util.h"
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

int main() {
    std::fstream file;
    std::string ignore_header;
    file.open(INPUT_FILE, std::ios::in);
    std::getline(file, ignore_header);
    StockTick sizeOfStockTick;
    size_t size = sizeof(StockTick[NUMBER_OF_ROW]);
    int output = open(OUT_FILE_PATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    truncate(OUT_FILE_PATH, size);
    void *points = mmap(nullptr, size, PROT_WRITE, MAP_SHARED, output, 0);
    StockTick *holders = (StockTick *)points;
    for (size_t i = 0; i < NUMBER_OF_ROW; i++) {
        holders[i] = getNextStockTick(file);
    }

    close(output);
    // dealocate point
    munmap(points, size);
    file.close();
    return 0;
}