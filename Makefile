all: control rules

control:
	g++ -std=c++17 -c src/usb-control.cpp -o build/usb-control.o
	g++ -std=c++17 -c src/main.cpp -o build/main.o
	g++ -std=c++17 -c src/exceptions.cpp -o build/exceptions.o
	g++ build/usb-control.o build/main.o build/exceptions.o -o bin/usb-control -l sqlite3 -lstdc++fs

rules:
	g++ -std=c++17 -c src/rules.cpp -o build/rules.o
	g++ -std=c++17 -c src/exceptions.cpp -o build/exceptions.o
	g++ build/rules.o build/exceptions.o -o bin/rules -l sqlite3 -lstdc++fs

clean:
	rm build/* database/* bin/*

run:
	sudo ./bin/usb-control

run_rules:
	./bin/rules $(ARGS)