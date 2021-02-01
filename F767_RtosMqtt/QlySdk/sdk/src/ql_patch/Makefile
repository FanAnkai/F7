SOURCES:= $(wildcard *.c)

all:$(SOURCES)
	$(CC) $(INCDIR) $(CFLAGS) -c $^
	$(AR) crv libiotpatch.a *.o
	mv libiotpatch.a $(LIBDIR)/
