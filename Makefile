CPP = g++
CPPFLAGS = -O2 -std=c++17 -Wall
CPPLIBS = -lc -lstdc++

ODIR = main/obj
CPPDIR = main
HDIR = $(CPPDIR)	# same as .cpp's

$(shell mkdir -p $(ODIR))

_DEPS = core.h
DEPS = $(patsubst %,$(HDIR)/%,$(_DEPS))

_OBJ = core.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CPPDIR)/%.cpp $(DEPS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

ndiag: $(OBJ)
	$(CPP) $(CPPFLAGS) $(CPPLIBS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ ndiag $(CPPDIR)/*~ 
	rmdir $(ODIR)