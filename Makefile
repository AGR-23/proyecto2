CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2
TARGET = filesystem
SRC = main.cpp FileSystem.cpp Metadata.cpp VersionGraph.cpp BlockManager.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) *bin *meta
	rm -rf storage.bin_metadata/

run:
	./$(TARGET)