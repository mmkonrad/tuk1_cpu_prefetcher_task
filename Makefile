main: main.cpp
	g++ main.cpp -std=c++14 -o main -lpthread -O3

clean:
	rm -f main
