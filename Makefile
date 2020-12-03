CC = g++

src = $(wildcard src/*.cpp) \
	$(wildcard grammar/MiniDecafLexer.cpp) \
	$(wildcard grammar/MiniDecafParser.cpp) \
	$(wildcard grammar/MiniDecafVisitor.cpp) \
	$(wildcard grammar/MiniDecafBaseVisitor.cpp)

CXXFLAGS = -Wall -g \
	-I/usr/local/include/antlr4-runtime \
	-Igrammar


LDFLAGS = -lantlr4-runtime

MiniDecaf: $(src) antlr
	$(CC) -o $@ $(src) $(CXXFLAGS) $(LDFLAGS)

.PHONY: antlr clean

antlr: grammar/CommonLex.g4 grammar/MiniDecaf.g4
	cd grammar && java -jar /usr/local/lib/antlr-4.8-complete.jar \
	-Dlanguage=Cpp -no-listener -visitor MiniDecaf.g4

clean:
	rm -f MiniDecaf
	rm -f grammar/*.cpp grammar/*.h grammar/*.interp grammar/*.tokens