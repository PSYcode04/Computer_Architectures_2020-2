CXX=g++


TARGET=mips-sim

OBJS = proj1.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

proj.o :
	$(CXX) -c proj1.cpp

.PHONY=clean

clean:
	rm -f $(OBJS) $(TARGET)

