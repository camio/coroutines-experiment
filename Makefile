.PHONY: all clean

CXX:= clang++
CXXFLAGS:=-std=c++20 # -stdlib=libc++
CPPFLAGS:=-fprebuilt-module-path=. -Istl_interfaces/include

all: main

clean:
	$(RM) main *.o *.pcm

%.pcm: %.cppm
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) --precompile $^ -o $@


%.o : %.pcm
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^ -o $@

# What follows is a failed attempt to use import <coroutine> instead of
# including it.
#
# %.pcm: %.cppm
#	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) -fmodule-file=coroutine.pcm --precompile $^ -o $@
#
# coroutine.pcm:
# $(CXX) $(CPPFLAGS) $(CXXFLAGS) -xc++-system-header --precompile coroutine -o $@
#
# corotest.cppm: coroutine.pcm

main.o: main.cpp corotest.pcm

main: main.o corotest.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
