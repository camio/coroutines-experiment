.PHONY: all clean

CXX := clang++
CXXFLAGS := -std=c++20
CPPFLAGS := -fprebuilt-module-path=. -Istl_interfaces/include

all: main

clean:
	$(RM) main *.o *.pcm

%.pcm: %.cppm
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) --precompile $< -o $@

%.o : %.pcm
	$(CXX) $(CXXFLAGS) -c $< -o $@

goro.input_iterator_coroutine_adapter.pcm: goro.lazy_cache_promise.pcm
goro.input_iterator_generator.pcm: goro.input_iterator_coroutine_adapter.pcm goro.lazy_cache_promise.pcm
goro.view_generator.pcm: goro.lazy_cache_promise.pcm goro.input_iterator_coroutine_adapter.pcm
goro.pcm: goro.input_iterator_coroutine_adapter.pcm goro.input_iterator_generator.pcm goro.lazy_cache_promise.pcm goro.view_generator.pcm

main.o: goro.pcm

main: main.o goro.o goro.lazy_cache_promise.o goro.input_iterator_coroutine_adapter.o goro.input_iterator_generator.o goro.view_generator.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
