BUILD_DIR=./build

all:
	g++ server.cpp -o $(BUILD_DIR)/server
	g++ client.cpp -o $(BUILD_DIR)/client

style:
	clang-format -i *.cpp
