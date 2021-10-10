# variables
#########################
CXX      := g++ -std=gnu++14
TARGET   := filesystem
CXXFLAGS := -w
DEPFLAGS := -MMD -MF $(@:.o=.d)
SRC      := src
BIN      := bin
CPP       = $(wildcard $(SRC)/*.cpp)
OBJ_RULE  = $(patsubst %.cpp, %.o, $(CPP)) #pattern, replacement, text
#########################

# make
#########################
all: setup $(TARGET) finish

setup:
	mkdir $(BIN)
	echo $(OBJ_RULE)

$(TARGET): $(OBJ_RULE)
	$(CXX) $(CXXFLAGS) -o $@ *.o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

finish:
	mv *.o $(BIN)
#########################

# clean
#########################
clean:
	rm -rf $(BIN)/
	rm $(TARGET)
#########################
