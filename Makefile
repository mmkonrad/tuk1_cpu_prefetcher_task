all: main bit_vector

main: main.cpp
	g++ main.cpp -std=c++14 -o main -lpthread -O3

bit_vector: BitVector.cpp
	g++ BitVector.cpp -std=c++14 -o bit_vector -mavx2 -O3

clean:
	rm -f main bit_vector
