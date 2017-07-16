FLAGS=-I./include -std=gnu++11
OBJFILES=u8char.o str.o console.o varcontainer.o objects.o actions.o parser.o game.o lamarr.o
OBJECTS=$(OBJFILES:%.o=objects/%.o)
LIBS=-lconio

debug.exe: $(OBJECTS)
	g++ $^ -o $@ $(LIBS)

objects/%.o: src/%.cpp include/%.h
	g++ $(FLAGS) $< -c -o $@

objects/%.o: src/%.c include/%.h
	g++ $(FLAGS) $< -c -o $@
