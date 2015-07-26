SOURCES	    :=  $(shell find src -name '*.cpp') $(shell find src -name '*.c')
OBJECTS	    :=  $(subst .c,.o,$(subst .cpp,.o,$(subst src/,build/,$(SOURCES))))
DIRECTORIES :=  $(sort $(dir $(OBJECTS)))

CXX				= g++
CC				= gcc
CFLAGS			= -O2 -g -fmessage-length=0
INCLUDE_PATHS	= -I../../lib/common -I../../lib/parse -I../../lib/parse_ucs -I../../lib/parse_expression -I../../lib/parse_prs -I../../lib/ucs -I../../lib/boolean -I../../lib/prs -I../../lib/interpret_ucs -I../../lib/interpret_boolean -I../../lib/interpret_prs
LIBRARY_PATHS	= -LC:\MinGW\bin -L../../lib/common -L../../lib/parse -L../../lib/parse_ucs -L../../lib/parse_expression -L../../lib/parse_prs -L../../lib/ucs -L../../lib/boolean -L../../lib/prs -L../../lib/interpret_ucs -L../../lib/interpret_boolean -L../../lib/interpret_prs
LIBRARIES		= -linterpret_prs -linterpret_boolean -linterpret_ucs -lprs -lboolean -lucs -lparse_prs -lparse_expression -lparse_ucs -lparse -lcommon
TARGET			= prsim

all: build $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LIBRARY_PATHS) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LIBRARIES)

build/%.o: src/%.cpp 
	$(CXX) $(INCLUDE_PATHS) $(CFLAGS) -c -o $@ $<

build/%.o: src/%.c 
	$(CC) $(INCLUDE_PATHS) $(CFLAGS) -c -o $@ $<

build:
	mkdir $(DIRECTORIES)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe
