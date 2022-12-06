CC   = gcc
EXEC = ACS
DEPS = acs_gen_2.h gpio.h rfx.h init.h
OBJS = acs_gen_2.o gpio.o rfx.o init.o acs.o

all: $(EXEC)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< 
 
$(EXEC): $(DEPS) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) x86-libm4api.a -lpthread
 
