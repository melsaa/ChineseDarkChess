all:
		g++ -g -std=c++11 -O3 -Wall main_cdc.cpp engine.cpp state.cpp magic.cpp search.cpp move_ordering.cpp -o cdc1


clean:
		rm -rf cdc
