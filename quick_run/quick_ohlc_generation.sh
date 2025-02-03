cd .. && cd build && \
./ohlc_generator \
--input_price_history_csv_file="../data/bitstamp_tick_data.csv" \
--output_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
&& \
./ohlc_generator \
--input_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--output_ohlc_history_binary_file="../data/bitstamp_tick_data_5min.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--interval_rate_sec=300 \
&& \
./ohlc_generator \
--input_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--output_ohlc_history_binary_file="../data/bitstamp_tick_data_30min.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--interval_rate_sec=1800 \
&& \
./ohlc_generator \
--input_price_history_binary_file="../data/bitstamp_tick_data.mov" \
--output_ohlc_history_binary_file="../data/bitstamp_tick_data_1h.mov" \
--start_time="2017-01-01" \
--end_time="2024-01-01" \
--interval_rate_sec=3600