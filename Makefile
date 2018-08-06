src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lm

ulm: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) ulm

