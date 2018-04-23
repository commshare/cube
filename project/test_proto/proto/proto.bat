set in_dir=D:\work\cube\project\test_proto\proto
set out_dir=D:\work\cube\project\test_proto\proto

"protoc.exe" --proto_path=%in_dir% --cpp_out=%out_dir% ^
riskapi_action_protocal.proto ^
riskapi_data_protocal.proto ^
quote_protocal.proto ^
trade_protocal.proto ^
trial_protocal.proto ^
monitor_protocal.proto ^
manager_protocal.proto ^
system_protocal.proto
pause