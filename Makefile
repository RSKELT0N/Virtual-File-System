
#########################
# variables
#########################
CXX      := g++
TARGET   := filesystem
CXXFLAGS := -std=gnu++17 -w
DEPFLAGS := -MMD -MF $(@:.o=.d)
SRC      := vfs_/src
BIN      := bin
DISKS    := disks
CPP       = $(wildcard $(SRC)/*.cpp)
OBJ_RULE  = $(patsubst %.cpp, %.o, $(CPP)) #pattern, replacement, text
#########################
# make
#########################
all: setup $(TARGET) finish

setup:
	mkdir $(BIN)
	mkdir $(DISKS)

$(TARGET): $(OBJ_RULE)
	$(CXX) $(CXXFLAGS) -pthread -o $@ *.o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

finish:
	mv *.o $(BIN)/
#########################
# clean
#########################
clean:
	rm -rf $(BIN)/
	rm -rf $(DISKS)/
	rm $(TARGET)
#########################
# rebuild
#########################
rb: clean all
#########################
