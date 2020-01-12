all:
		g++ -g -std=c++11 -O3 -Wall main_cdc.cpp engine.cpp state.cpp magic.cpp -o cdc


clean:
		rm -rf cdc
