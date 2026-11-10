CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lboost_program_options -lcryptopp

APP_SRCS = main.cpp Application.cpp CLIHandler.cpp ClientServer.cpp CryptoAuthModule.cpp DataReader.cpp
APP_OBJS = $(APP_SRCS:.cpp=.o)
APP_TARGET = client

TEST_SRCS = tests.cpp CLIHandler.cpp ClientServer.cpp CryptoAuthModule.cpp DataReader.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = run_tests

all: $(APP_TARGET)

$(APP_TARGET): $(APP_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $^ $(LIBS) -lUnitTest++

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(APP_OBJS) $(APP_TARGET)

clean_tests:
	rm -f $(TEST_OBJS) $(TEST_TARGET) *.conf *.bin *.txt

distclean: clean clean_tests
