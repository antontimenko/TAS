CFLAGS=-std=c11 -Wall -Wextra -pedantic
CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic
LIBS=
SOURCES= \
	main.cpp \
	Integer.cpp \
	Compiler.cpp \
	Lexeme.cpp \
	Exception.cpp \
	Token.cpp \
	Diagnostics.cpp \
	Preprocessor.cpp \
	Math.cpp \
	PseudoSentence.cpp \
	Instruction.cpp \
	RawSentence.cpp \
	Sentence.cpp

all: build_dir tas

build_dir:
	mkdir -p build dep

tas: $(addprefix build/,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES))))
	clang++ -o build/$@ $^ $(LIBS)

build/%.o: src/%.c
	clang -c -o $@ $< $(CFLAGS)
	clang -MM -MF dep/$*.d -MT $@ $< $(CFLAGS)

build/%.o: src/%.cpp
	clang++ -c -o $@ $< $(CXXFLAGS)
	clang++ -MM -MF dep/$*.d -MT $@ $< $(CXXFLAGS)

-include $(addprefix dep/,$(patsubst %.c,%.d,$(patsubst %.cpp,%.d,$(SOURCES))))

clean:
	rm -Rf build dep
