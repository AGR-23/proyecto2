CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
TARGET = FileSystem
SRC = main.cpp FileSystem.cpp Metadata.cpp VersionGraph.cpp BlockManager.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) *bin

run:
	./$(TARGET)