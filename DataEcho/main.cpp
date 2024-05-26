#include "../src/StockTick.h"
#include "example-config.h"
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
    StockTick sizeOfStockTick;
    size_t size = sizeof(StockTick[NUMBER_OF_ROW]);
    int input = open(INPUT_FILE_PATH, O_RDONLY);
    void *points = mmap(nullptr, size, PROT_READ, MAP_SHARED, input, 0);
    StockTick *holders = (StockTick *)points;
    StockTick holder;
    for (size_t i = 0; i < NUMBER_OF_ROW; i++) {
        holder = holders[i];
        std::time_t time = std::chrono::system_clock::to_time_t(holder.date);
        std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "\n";
    }
    close(input);
    std::string stopper;
    while (std::cin >> stopper) {
        std::cout << stopper << "\n";
    }
    munmap(points, size);
    return 0;
}