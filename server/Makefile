CC = g++
LIBFLAGS =
CPPFLAGS = -pthread -std=c++17 -Wall -Wshadow -Wpedantic -O2

DEPS =
OBJECTS = server.o
PROJECT = server

%.o : %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

$(PROJECT): $(OBJECTS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBFLAGS)

clean:
	rm $(PROJECT) $(OBJECTS)
