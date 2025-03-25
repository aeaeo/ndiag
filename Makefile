CPP = g++
CPPFLAGS = -O2 -std=c++17 -Wall
CPPLIBS = -lc -lstdc++

TARGET = ndiag
CPPDIR = main
HDIR = $(CPPDIR)
ODIR = main/obj

$(shell mkdir -p $(ODIR))

_DEPS = core.h
DEPS = $(patsubst %,$(HDIR)/%,$(_DEPS))

_OBJ = core.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CPPDIR)/%.cpp $(DEPS)
	@echo "Compiling $<"
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	@echo "Linking the executable"
	$(CPP) $(CPPFLAGS) $(CPPLIBS) -o $@ $^

.PHONY: clean setcaps

clean:
	@echo "Cleaning up"
	rm -f $(ODIR)/*.o *~ $(TARGET) $(CPPDIR)/*~ 
	rmdir $(ODIR)

setcaps:
	@echo "Setting CAP_NET_RAW"
	sudo setcap cap_net_raw=ep $(TARGET)