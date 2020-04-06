all:
	g++ -std=c++17 usb-control.cpp -o usb-control -l sqlite3
	g++ rules.cpp -o rules -l sqlite3

run:
	./bus-control