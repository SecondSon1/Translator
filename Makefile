CC = clang++
SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:.cpp=.o)
BIN = bin

#INCFLAGS = -I/opt/homebrew/Cellar/sfml/2.5.1_1/include
#LIBFLAGS = -L/opt/homebrew/Cellar/sfml/2.5.1_1/lib -lsfml-system -lsfml-audio -lsfml-graphics -lsfml-network -lsfml-window
INCFLAGS=
LIBFLAGS=
CCFLAGS = -std=c++17 -Wextra -Wshadow -Wconversion -Wfloat-equal -O2
#CCFLAGS += -fsanitize=undefined,bounds,address

all: build

dirs:
	mkdir -p ./$(BIN)

%.o: %.cpp
	$(CC) -o $@ -c $< $(INCFLAGS) $(CCFLAGS)

build: clean dirs $(OBJ)
	$(CC) -o $(BIN)/app $(filter %.o,$^) -lm $(LIBFLAGS) $(CCFLAGS)

run: build
	$(BIN)/app

clean:
	rm -rf $(OBJ)
	rm -rf $(BIN)
