FLAGS=-I./include -std=gnu++11 -static-libgcc -static-libstdc++
OBJFILES=u8char.o str.o console.o varcontainer.o objects.o actions.o parser.o game.o lamarr.o
OBJECTS=$(OBJFILES:%.o=objects/%.o)
LIBS=-lconio

hedy.exe: $(OBJECTS) icon.res
	g++ $^ -o $@ $(LIBS) -s

icon.res: icon.rc
	windres icon.rc -O coff -o icon.res

objects/%.o: src/%.cpp include/%.h
	g++ $(FLAGS) $< -c -o $@

objects/%.o: src/%.c include/%.h
	g++ $(FLAGS) $< -c -o $@
