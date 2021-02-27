CXX=g++


TARGET=mips-sim

OBJS = proj3.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

proj.o :
	$(CXX) -c proj3.cpp

.PHONY=clean

clean:
	rm -f $(OBJS) $(TARGET)