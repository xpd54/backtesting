# backtesting

Convert csv to binary

```
./ohlc_generator \
--input_price_history_csv_file="../data/bitstamp_tick_data.csv" \
--output_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01"
```

Convert TPV to OHLC with 5 min frequency rate

```
./ohlc_generator \
--input_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--output_ohlc_history_binary_file="../data/bitstamp_tick_data_5min.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--interval_rate_sec=300
```

Convert TPV to OHLC with 1h frequency rate

```
./ohlc_generator \
--input_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--output_ohlc_history_binary_file="../data/bitstamp_tick_data_1h.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--interval_rate_sec=3600
```

Run Trade Simulation on 5 min frequency

```
./trade_simulator \
--input_price_history_binary_file="../data/bitstamp_tick_data_5min.mov" \
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--start_base_balance=1.0 \
--start_quote_balance=0.0
```

Run Trade Simulation on 1 hour frequency

```
./trade_simulator \
--input_price_history_binary_file="../data/bitstamp_tick_data_1h.mov" \
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--start_base_balance=1.0 \
--start_quote_balance=0.0
```

Run Trade Simulation on 1 hour frequency for multiple 6 month period

```
./trade_simulator \
--input_price_history_binary_file="../data/bitstamp_tick_data_1h.mov" \
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--evaluation_period_months=6 \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--start_base_balance=1.0 \
--start_quote_balance=0.0
```

Run Trade Simulation on 1 hour frequency for multiple 6 month period with all combination
(Getting best alpha and epsiolon sorted with score)

```
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
./plot --output_account_log_file="../data/account.log"
```
