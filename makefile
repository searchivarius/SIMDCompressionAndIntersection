.SUFFIXES:
#
.SUFFIXES: .cpp .o .c .h
# replace the YOURCXX variable with a path to a C++11 compatible compiler.
ifeq ($(INTEL), 1)
# if you wish to use the Intel compiler, please do "make INTEL=1".
    YOURCXX ?= /opt/intel/bin/icpc
ifeq ($(DEBUG),1)
    CXXFLAGS = -std=c++11 -O3 -Wall -ansi -xAVX -DDEBUG=1 -D_GLIBCXX_DEBUG   -ggdb
else
    CXXFLAGS = -std=c++11 -O3 -Wall -ansi -xAVX -DNDEBUG=1  -ggdb
endif # debug
else #intel
    YOURCXX ?= g++-4.7
ifeq ($(DEBUG),1)
    CXXFLAGS = -mavx -std=c++11  -Weffc++ -pedantic -ggdb -DDEBUG=1 -D_GLIBCXX_DEBUG -Wall -Wextra  -Wcast-align  
else
    CXXFLAGS = -mavx -std=c++11  -Weffc++ -pedantic -O3 -Wall -Wextra  -Wcast-align  
endif #debug
endif #intel

CXX := $(YOURCXX)




HEADERS= $(shell ls include/*h)

all: unit  testcodecs  testintegration  advancedbenchmarking benchintersection
	echo "please run unit tests by running the unit executable"

advancedbenchmarking: simplesynth compress uncompress budgetedtest entropy 

ironpeterpackinghelpers.o: include/ironpeterpackinghelpers.h src/ironpeterpackinghelpers.cpp
	$(CXX) $(CXXFLAGS) -c src/ironpeterpackinghelpers.cpp -Iinclude

bitpacking.o: include/bitpacking.h src/bitpacking.cpp
	$(CXX) $(CXXFLAGS) -c src/bitpacking.cpp -Iinclude

intersection.o: include/intersection.h src/intersection.cpp
	$(CXX) $(CXXFLAGS) -c src/intersection.cpp -Iinclude

benchintersection: intersection.o src/benchintersection.cpp include/synthetic.h include/timer.h
	$(CXX) $(CXXFLAGS) -o benchintersection src/benchintersection.cpp intersection.o -Iinclude

likwidintersection: intersection.o src/benchintersection.cpp include/synthetic.h include/timer.h
	$(CXX) $(CXXFLAGS) -DLIKWID_MARKERS -pthread -o likwidintersection src/benchintersection.cpp intersection.o -Iinclude -llikwid 

integratedbitpacking.o: include/integratedbitpacking.h src/integratedbitpacking.cpp 
	$(CXX) $(CXXFLAGS) -c src/integratedbitpacking.cpp -Iinclude


simdbitpacking.o: include/simdbitpacking.h src/simdbitpacking.cpp
	$(CXX) $(CXXFLAGS) -c src/simdbitpacking.cpp -Iinclude

usimdbitpacking.o: include/usimdbitpacking.h src/usimdbitpacking.cpp
	$(CXX) $(CXXFLAGS) -c src/usimdbitpacking.cpp -Iinclude

simdintegratedbitpacking.o: include/simdintegratedbitpacking.h include/delta.h src/simdintegratedbitpacking.cpp
	$(CXX) $(CXXFLAGS) -c src/simdintegratedbitpacking.cpp -Iinclude



UNAME := $(shell uname)


OBJECTS= bitpacking.o integratedbitpacking.o simdbitpacking.o usimdbitpacking.o    simdintegratedbitpacking.o   intersection.o  ironpeterpackinghelpers.o


unit: $(HEADERS)   src/unit.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o unit src/unit.cpp  $(OBJECTS) -Iinclude

testcodecs: $(HEADERS) src/testcodecs.cpp   $(OBJECTS)
	$(CXX) $(CXXFLAGS) -Iinclude -o testcodecs src/testcodecs.cpp   $(OBJECTS)



example:  $(HEADERS) example.cpp  $(OBJECTS)
	$(CXX) $(CXXFLAGS)  -o example example.cpp  $(OBJECTS) -Iinclude



testintegration:  bitpacking.o simdbitpacking.o usimdbitpacking.o integratedbitpacking.o     simdintegratedbitpacking.o src/testintegration.cpp  $(HEADERS) 
	$(CXX) $(CXXFLAGS) -Iinclude -o testintegration src/testintegration.cpp   bitpacking.o integratedbitpacking.o  simdbitpacking.o usimdbitpacking.o     simdintegratedbitpacking.o 



clean: 
	rm -f *.o unit testintegration testcodecs   simplesynth  compress uncompress budgetedtest   entropy example benchintersection






BENCHHEADERS= $(shell ls advancedbenchmarking/include/*h)

simplesynth: $(HEADERS) $(BENCHHEADERS) advancedbenchmarking/src/simplesynth.cpp 
	$(CXX) $(CXXFLAGS) -o simplesynth advancedbenchmarking/src/simplesynth.cpp  -Iinclude -Iadvancedbenchmarking/include


compress: $(HEADERS) $(BENCHHEADERS) advancedbenchmarking/src/compress.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o compress advancedbenchmarking/src/compress.cpp $(OBJECTS) -Iinclude -Iadvancedbenchmarking/include

budgetedtest: $(HEADERS) $(BENCHHEADERS) advancedbenchmarking/src/budgetedtest.cpp $(OBJECTS) 
	$(CXX) $(CXXFLAGS) -o budgetedtest advancedbenchmarking/src/budgetedtest.cpp $(OBJECTS)  -Iinclude -Iadvancedbenchmarking/include

entropy: $(HEADERS) $(BENCHHEADERS) advancedbenchmarking/src/entropy.cpp $(OBJECTS) 
	$(CXX) $(CXXFLAGS) -o entropy advancedbenchmarking/src/entropy.cpp $(OBJECTS)  -Iinclude -Iadvancedbenchmarking/include


uncompress: $(HEADERS) $(BENCHHEADERS) advancedbenchmarking/src/uncompress.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o uncompress advancedbenchmarking/src/uncompress.cpp $(OBJECTS) -Iinclude -Iadvancedbenchmarking/include 

astyle:
	astyle --options=astyle.conf --recursive "*.cpp" "*.h"

.PHONY: all clean astyle
