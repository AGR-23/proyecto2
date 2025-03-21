CXX = g++ 
CXXFLAGS = -std=c++17 -Wall 

TARGET = versionedFileApp

SRC = main.cpp versionedFile.cpp 
OBJ = $(SRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	 valgrind --leak-check=full ./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) *.txt