CC = gcc
LD = gcc
INCPATHS = -Iopenssl/include -Iwinscard/include
CFLAGS = -W -Wno-write-strings -Wno-unused-function -O4 -DNDEBUG $(INCPATHS)
LIBS = openssl/lib/ssleay32.a openssl/lib/libeay32.a winscard/lib/WinSCard.Lib

OBJS = iclassified.o
EXES = iclass.exe

all: $(OBJS) $(EXES)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.exe : %.c $(OBJS)
	$(LD) $(CFLAGS) $(LDFLAGS) -o $@ $< $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(EXES)
