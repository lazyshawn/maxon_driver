# List of demos to compile and link
DEVLIST=002 003 005 101 102 105 109 10A 10E 111 113 116
LIBLIST=002DriverLinux 003DriverLinux 005DriverLinux 101DriverLinux 102DriverLinux 105DriverLinux 109DriverLinux 10ADriverLinux 10EDriverLinux 111DriverLinux 113DriverLinux-usb-1.0 116DriverLinux-usb-1.0

CXXFLAGS=-I. -I../../inc
CLFLAGS=$(LIBLIST:%=-leci%)
CXX=gcc

default: LinuxEciDemo

LinuxEciDemo: $(DEVLIST:%=EciDemo%.o) EciDemoCommon.o LinuxEciDemo.o
	$(CXX) -o ../../bin/release/LinuxEciDemo LinuxEciDemo.o EciDemoCommon.o $(DEVLIST:%=EciDemo%.o) $(CLFLAGS)

LinuxEciDemo.o: LinuxEciDemo/LinuxEciDemo.c
	$(CXX) -c $(CXXFLAGS) LinuxEciDemo/LinuxEciDemo.c
  
EciDemoCommon.o: EciDemoCommon.c EciDemoCommon.h
	$(CXX) -c $(CXXFLAGS) EciDemoCommon.c

$(DEVLIST:%=EciDemo%.o): $(DEVLIST:%=EciDemo%.c) $(DEVLIST:%=EciDemo%.h)
	$(CXX) -c $(CXXFLAGS) $(DEVLIST:%=EciDemo%.c)
  
clean:
	rm -f *.o ../../bin/release/LinuxEciDemo
