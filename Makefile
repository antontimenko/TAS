CFLAGS=-std=c11 -Wall -Wextra -pedantic
CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic
LIBS=
SOURCES=main.cpp TSCompiler.cpp

all: build_dir tas

build_dir:
	mkdir -p build
	mkdir -p dep

tas: $(addprefix build/,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))
	clang++ -o build/$@ $^ $(LIBS)

build/%.o: src/%.c
	clang -c -o $@ $< $(CFLAGS)
	clang -MM -MF dep/$*.d -MT $@ $<

build/%.o: src/%.cpp
	clang++ -c -o $@ $< $(CXXFLAGS)
	clang++ -MM -MF dep/$*.d -MT $@ $<

-include $(addprefix dep/,$(patsubst %.c,%.d,$(patsubst %.cpp,%.d,$(SOURCES))))

clean:
	rm -R build
	rm -R dep
