#pragma once
#include "../../../src/StockTick.h"
#include <fstream>
StockTick getNextStockTick(std::fstream &stock_data_stream);