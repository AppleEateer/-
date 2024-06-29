CXX = g++
CXXFLAGS = -std=c++11 -Wall

TARGET = zip
SRC = zip.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -lz

clean:
	rm -f $(TARGET)