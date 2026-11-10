CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lboost_program_options -lcryptopp

SRCS = main.cpp Application.cpp CLIHandler.cpp DataReader.cpp CryptoAuthModule.cpp ClientServer.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = client

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
