CXX=g++


TARGET=mips-sim

OBJS = proj2.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

proj.o :
	$(CXX) -c proj2.cpp

.PHONY=clean

clean:
	rm -f $(OBJS) $(TARGET)