CXX := gcc
CXXFLAGS = -I./common -O3 -pg -Wall -lpthread -mavx -mavx2 -march=native
all: pi

pi: pi.c
	$(CXX) pi.c $(CXXFLAGS) -o ./pi.out

.PHONY : clean

clean:
	rm -f *.out pi