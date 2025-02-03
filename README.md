# backtesting

Convert csv to binary

```
./ohlc_generator \
--input_price_history_csv_file="../data/bitstampUSD.csv" \
--output_price_history_binary_file="../data/bitstampUSD.mov" \
--start_time="2017-01-01" \
--end_time="2022-01-01"
```

Convert TPV to OHLC with 5 min frequency rate

```
./ohlc_generator \
--input_price_history_binary_file="../data/bitstampUSD.mov" \
--output_ohlc_history_binary_file="../data/bitstampUSD_5min.mov" \
--start_time="2017-01-01" \
--end_time="2022-01-01" \
--interval_rate_sec=300
```

Convert TPV to OHLC with 1h frequency rate

```
./ohlc_generator \
--input_price_history_binary_file="../data/bitstampUSD.mov" \
--output_ohlc_history_binary_file="../data/bitstampUSD_1h.mov" \
--start_time="2017-01-01" \
--end_time="2022-01-01" \
--interval_rate_sec=3600
```

```
./trade_simulator \
--input_price_history_binary_file="../data/bitstampUSD_5min.mov" \
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--start_time="2017-01-01" \
--end_time="2022-01-01" \
--start_base_balance=1.0 \
--start_quote_balance=0.0
```
