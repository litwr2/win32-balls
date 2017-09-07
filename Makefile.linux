#for Linux with MinGW
CC		= /opt/mxe/usr/bin/i686-pc-mingw32-gcc
CXX		= /opt/mxe/usr/bin/i686-pc-mingw32-g++
RESC		= /opt/mxe/usr/bin/i686-pc-mingw32-windres

CFLAGS1		= -s -O3 -mwindows -posix
CFILE		= balls

CCOPTIONS	= $(CFLAGS1)
LDINIOPTS	= -O3 -s -posix -static
LDOPTIONS	= -mwindows $(LDINIOPTS)
#-Wl,-e_WinMain@16

EXTRA_LIBRARIES	= -lstdc++

all: $(CFILE).exe

$(CFILE).exe: $(CFILE).o
	$(CXX) -o $@ $(LDOPTIONS) $< $(EXTRA_LIBRARIES)

$(CFILE).o: $(CFILE).cpp
	$(CXX) -c -o $@ $(FLAGS1) $<

clean::
	rm -f *.o *.exe
