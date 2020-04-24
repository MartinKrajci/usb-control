all: control rules

control:
	g++ -std=c++17 -c src/usb-control.cpp -o build/usb-control.o
	g++ -std=c++17 -c src/main.cpp -o build/main.o
	g++ build/usb-control.o build/main.o -o bin/usb-control -l sqlite3 -lstdc++fs

rules:
	g++ -c src/rules.cpp -o build/rules.o
	g++ build/rules.o -o bin/rules -l sqlite3

clean:
	rm build/usb-control.o build/main.o build/rules.o

run:
	sudo ./bin/usb-control

run_rules:
	./bin/rules $(ARGS)