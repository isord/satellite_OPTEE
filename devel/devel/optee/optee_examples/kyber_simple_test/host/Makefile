CC      ?= $(CROSS_COMPILE)gcc
LD      ?= $(CROSS_COMPILE)ld
AR      ?= $(CROSS_COMPILE)ar
NM      ?= $(CROSS_COMPILE)nm
OBJCOPY ?= $(CROSS_COMPILE)objcopy
OBJDUMP ?= $(CROSS_COMPILE)objdump
READELF ?= $(CROSS_COMPILE)readelf

OBJS = main.o

CFLAGS += -Wall -Wbad-function-cast -Wcast-align \
          -Werror-implicit-function-declaration -Wextra \
          -Wfloat-equal -Wformat-nonliteral -Wformat-security \
          -Wformat=2 -Winit-self -Wmissing-declarations \
          -Wmissing-format-attribute -Wmissing-include-dirs \
          -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs \
          -Wpointer-arith -Wshadow -Wstrict-prototypes \
          -Wswitch-default -Wunsafe-loop-optimizations \
          -Wwrite-strings -Werror

#Add/link other required libraries here
LDADD += -lteec -lpthread

BINARY = kyber_test

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $< $(LDADD)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
