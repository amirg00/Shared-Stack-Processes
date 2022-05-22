#!make -f
CXX=clang++-9
CXXFLAGS=-std=c++2a

OBJECTS=$(subst .cpp,.o,$(SOURCES))
SOURCES=Stack.cpp

all: server client

run:
	./$^

server: server.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o server

test: TestCounter.o Test.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o test

%.o: %.cpp
	$(CXX) $(CXXFLAGS) --compile $< -o $@

tidy:
	clang-tidy $(SOURCES) -checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,performance-*,portability-*,readability-* --warnings-as-errors=* --

clean:
	rm -f *.o test server client
	rm -f StudentTest*.cpp