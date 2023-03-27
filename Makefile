CC := gcc
CFLAGS := -Wall

.PHONY: run clean

run: rvc_cli
	./rvc_cli

clean:
	rm ./*.o
	rm ./rvc_cli

rvc_cli: main.o
	$(CC) $^ -o $@

%.c: %.o
	$(CC) $(CFLAGS) -c $^ -o $@
