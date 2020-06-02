CXX = g++
CXXFLAGS = -std=c++17
LIBS = -l sqlite3 -lstdc++fs

all: ./bin/usb-control ./bin/rules

./bin/usb-control: ./build/usb-control.o ./build/main.o ./build/exceptions.o
	$(CXX) build/usb-control.o build/main.o build/exceptions.o -o bin/usb-control $(LIBS)

./build/usb-control.o: ./src/usb-control.cpp
	$(CXX) $(CXXFLAGS) -c src/usb-control.cpp -o build/usb-control.o

./build/main.o: ./src/main.cpp
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o build/main.o

./bin/rules: ./build/rules.o ./build/exceptions.o
	$(CXX) build/rules.o build/exceptions.o -o bin/rules $(LIBS)

./build/rules.o: ./src/rules.cpp
	$(CXX) $(CXXFLAGS) -c src/rules.cpp -o build/rules.o

./build/exceptions.o: ./src/exceptions.cpp
	$(CXX) $(CXXFLAGS) -c src/exceptions.cpp -o build/exceptions.o

clean:
	rm build/* database/* bin/*

run:
	sudo ./bin/usb-control

run_rules:
	./bin/rules $(ARGS)