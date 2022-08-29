CC = g++
LIBFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
CPPFLAGS = -pthread -std=c++17 -Wall -Wshadow -Wpedantic -O2

DEPS = login.hpp title.hpp chat.hpp Activity.hpp
OBJECTS = client.o
PROJECT = client

%.o : %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

$(PROJECT): $(OBJECTS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBFLAGS)

clean:
	rm $(PROJECT) $(OBJECTS)
