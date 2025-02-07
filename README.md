# Backtesting

Backtesing framework written in C++17 to simulate trading strategy for BTC/USD pair.

#### DISCLAIMER ⚠️🛑
> [!WARNING]
THIS FRAMEWORK IS NOT INTENDED FOR ANY REAL-WORLD INVESTMENT DECISIONS. THE USE OF THIS FRAMEWORK AND THE DATA PROVIDED IS AT YOUR OWN RISK. I DO NOT TAKE ANY RESPONSIBILITY FOR THE CORRECTNESS OF THE DATA OR ANY OUTPUTS GENERATED BY THIS FRAMEWORK WHEN APPLIED IN REAL-WORLD SCENARIOS. THIS FRAMEWORK IS DESIGNED SOLELY FOR LEARNING AND EDUCATIONAL PURPOSES.

## Table of Contents
- [Dependency](#dependency)
- [Project Structure](#project-structure)
- [Data Download](#data-download)
- [Compiling & Building](#compiling-building)
- [Tick Data Generation](#tick-data-generation)
- [Strategy Example And Other Terms](#strategy-and-terms)
- [How to Run](#how-to-run)
- [How To Add New Strategy](#how-to-add-new-strategy)
- [Performance Details](#performance-details)
- [Task & Improvements](#task-and-improvements)
- [License](#license)

#### Dependency

[common-util](https://github.com/xpd54/common_util) added as git submodule.
[gnu plot](http://www.gnuplot.info/) On mac easy installation would be `brew install gnuplot`
[googletest](https://github.com/google/googletest) for unit test cases.
C++17,
CMake,
Clang,

#### Project-Structure

```bash
├── backtesting
│   ├── base (This compiles as static lib which get used in both data generation and simulation)
│   │   ├── account (Keep track of account balance/simulate buy and sell)
│   │   ├── common_interface
│   │   ├── price_history (OHLC data clean up)
│   │   ├── trade_simulator (Interface to add new trade strategy)
│   │   └── util (Read/Write into binary file, getting cmd line argument)
│   ├── execution (actually running the simulation)
│   ├── logs (log generation which get used to plot a graph)
│   └── simulators (Trading strategy and Trade Simulator logic)
│       └── strategy
├── data (Create this folder to hold ohlc data)
├── data_generator (Logic to convert TPV to OHLC and change frequency of OHLC)
├── external (Added dependancy as git submodule, memory map file, logger, time, command line argument and string formate util)
├── quick_run (bash script to quickly running the project)
└── result_plot (logic to plot a graph after running simulation)
```

#### Data-Download

Time Price Volume (TPV) data is hosted on [kaggle](https://www.kaggle.com/datasets/saltwateroil/bitstamp-tpv-usd). It can be downloaded directly or with use of `wget` 
[link to download](https://www.kaggle.com/api/v1/datasets/download/saltwateroil/bitstamp-tpv-usd).

Create a folder named `data` and unzip data there with the name `bitstamp_tick_data.csv`<br>
![hardware](/screenshots/data-download.png)

#### Compiling-Building

1. clone the repo `git clone https://github.com/xpd54/backtesting`
2. update submodule `git submodule update --init --recursive`
3. create a build folder and build `mkdir build && cd build && cmake .. && make`

This generate 3 exicutables

1. ohlc_generator (To convert TPV to binary form of OHLC data formate)
2. trade_simulator (Execute trade simulation)
3. plot (plot graph with evaluation log)

#### Tick-Data-Generation

Any trade simulation we would like to run on different frequency of tick data. `ohlc_generator` does this to do it quickly with **[start_time - end_time)** (time range to consider for ohlc (open high low close))

`cd quick_run && chmod +x quick_ohlc_generation.sh`
`./quick_ohlc_generation.sh` (might take a minute or two as tpv data is close to 3.2gb). I suggest checking `quick_ohlc_generation.sh` before running it, naming convention is self explanatory.

#### Strategy-and-Terms

Rebalancing Trade Strategy:- The basic idea here is to keep the portfolio value (BTC \* BTC price + USD) in a way where cryptocurrency weight is defined by **alpha** and it's allowed to go up and down by **epsilon**.
Example:-
**alpha** 0.7 means that we would like to keep 70% of the total portfolio value in the base (crypto) currency,
and 30% in the quote currency (USD) for whole execution time. So Current quick run between
**[start_time='2017-01-01' - end_time='2024-01-01']**
we would maintain 70% in BTC and 30% in USD for the whole 7 years while looking to make a profit.

**epsilon** 0.05f means the maximum allowed deviation from the desired alpha allocation.
We allow the actual base (crypto) currency allocation to be within the interval: **[alpha x (1 - epsilon) - alpha x (1 + epsilon)]**.

So [0.735 - 0.665] which means it's allowed to allocate 73.5% to 66.5% in BTC and the rest corresponding in USD.
<small>codebase is self-explanatory but will add more clarity in words soon!!</small>

#### How-to-Run

use `quick_run/quick_simulation.sh` to run trade simulation over 5 min frequency data.
Example commands are [HERE](https://github.com/xpd54/backtesting/tree/main/quick_run)

```bash
./trade_simulator \
--input_price_history_binary_file="../data/bitstamp_tick_data_5min.mov" \ (change it to 30min.mov and 1h.mov for other frequency)
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--start_time="2017-01-01" \ (start execution from jan 2017 to jan 2024)
--end_time="2024-01-01" \
--start_base_balance=1.0 \ (we have 1 btc to start with)
--start_quote_balance=0.0 \ (we have 0 usd to start with)
```

Here we can see with the rebalancing trade strategy we have a gain of **2043.60%**. But if we would have just bought and hold it would have been **4272.85%**. Well HODL strategy is the best in the crypto market do time travel and buy in 2011 and sell in 2024 that's [HODL](https://www.investopedia.com/terms/h/hodl.asp).

```
[2025-02-03 20:47:00.665] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.7000|0.0500] evaluation
[2025-02-03 20:47:05.889] [0x7ff8599feb40] [INFO] --------------- Time Period ---------------  Strategy gain | Base gain(HODL) | Score | volatility
[2025-02-03 20:47:05.890] [0x7ff8599feb40] [INFO] [2017-01-01 00:00:00 - 2024-01-01 00:00:00):  2043.6094%  | 4272.8594%  | 0.4902  | 0.0000  | 0.0000
[2025-02-03 20:47:05.895] [0x7ff8599feb40] [INFO] Evaluated in 0:0:5sec
```

if we run the same strategy over diff frequencies it should be close enough (2055% gain) on 1h frequency with a similar 0.49 score.

```
[2025-02-03 20:55:00.884] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.7000|0.0500] evaluation
[2025-02-03 20:55:01.650] [0x7ff8599feb40] [INFO] --------------- Time Period ---------------  Strategy gain | Base gain(HODL) | Score | volatility
[2025-02-03 20:55:01.650] [0x7ff8599feb40] [INFO] [2017-01-01 00:00:00 - 2024-01-01 00:00:00):  2055.8738%  | 4271.8188%  | 0.4931  | 0.0000  | 0.0000
[2025-02-03 20:55:01.657] [0x7ff8599feb40] [INFO] Evaluated in 0:0:1sec
```

We should look into our evaluation log which shows we are selling our 0.3 BTC immediately where our alpha was set for 0.7 (70% in BTC) we achieved that balance.

```
1483228800,966.3400,966.9900,964.6000,966.6000,102.4848,1.0000,0.0000,0.0000,,,,,
1483232400,966.6000,966.6000,962.5400,963.8700,149.0256,1.0000,0.0000,0.0000,,,,,
1483232400,966.6000,966.6000,962.5400,963.8700,149.0256,0.7000,287.9200,1.4500,MARKET,SELL,0.300000,,
1483236000,964.3500,965.7500,961.9900,963.9700,94.2674,0.7000,287.9200,1.4500,,,,,
```

If we plot the portfolio value we see it started with 1 BTC and goes down (get sold and allocate in USD) asap after that it trades in the range of [0.735 - 0.665] (73.5% to 66.5%) of allocation.
(The number of data points that I am using is limited to 30000 as higher data point plot generation becomes slower)
![Plot](/screenshots/gnuplot.png)

Let's run it with multiple combinations of alpha and epsilon **[alpha|epsilon]**. Again if we look int the score we can see highest score suggest allocating most of the assets in BTC which is HODL (buy and hold).

```bash
./trade_simulator \
--input_price_history_binary_file="../data/bitstamp_tick_data_1h.mov" \
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--evaluation_period_months=6 \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--start_base_balance=1.0 \
--start_quote_balance=0.0 \
--evaluate_combination=1
```

```
[2025-02-03 21:05:18.790] [0x7ff8599feb40] [INFO] Evaluation Combination of simulators
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.9000|0.2000]: 1.0000
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.9000|0.1000]: 0.9948
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.9000|0.0500]: 0.9905
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.9000|0.0100]: 0.9812
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.7000|0.2000]: 0.9586
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.7000|0.0500]: 0.9509
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.7000|0.1000]: 0.9483
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.7000|0.0100]: 0.9421
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.5000|0.1000]: 0.9132
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.5000|0.2000]: 0.9126
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.5000|0.0500]: 0.9085
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.5000|0.0100]: 0.8923
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.3000|0.2000]: 0.8647
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.3000|0.1000]: 0.8625
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.3000|0.0500]: 0.8556
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.3000|0.0100]: 0.8429
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.1000|0.2000]: 0.8064
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.1000|0.1000]: 0.8048
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.1000|0.0500]: 0.8024
[2025-02-03 21:05:18.886] [0x7ff8599feb40] [INFO] rebalancing_trade_simulator[0.1000|0.0100]: 0.7963
```

<sub>TODO:- Add more details on how rebalancing trade strategy works</sub>

#### How-To-Add-New-Strategy

By implementation of an interface provided into `backtesting/base/trade_simulator/trade_simulator.hpp`
And updating `backtesting/simulators/simulator_factory.hpp` and `backtesting/simulators/strategy/common_strategy_config.hpp`
it's easily can be achieved.

#### Performance-Details

I am using [mmap](https://man7.org/linux/man-pages/man2/mmap.2.html) to read and write into the file. Which map the disk file directly to memory. I wrote a [common-util](https://github.com/xpd54/common_util) which have easy to use header-only lib.

#### Task-And-Improvements

- [x] Single instance Logger for both console and logfile.
- [ ] Use CRTP to avoid using virtual.
- [x] Improve read/write with memory mapped file.
- [x] Write OHLC data to a binary file.
- [ ] Run account logging on different processes.
- [ ] Rebalancing trade strategy doc.
- [ ] Financial indicators
- [x] Integrate google test
- [ ] Unit Test cases
