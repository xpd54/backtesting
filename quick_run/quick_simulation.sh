cd .. && cd build && \
./trade_simulator \
--input_price_history_binary_file="../data/bitstamp_tick_data_5min.mov" \
--output_account_log_file="../data/account.log" \
--output_simulator_log_file="../data/simulator.log" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--start_base_balance=1.0 \
--start_quote_balance=0.0 \
&& \
./plot --output_account_log_file="../data/account.log"