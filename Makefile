CC := gcc
CFLAGS := -Wall -std=c99 -pedantic

rvc_cli: main.o
	$(CC) $^ -o $@

%.c: %.o
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: run clean rebuild test_progs

test_progs:
	make -C test_progs all

run: rvc_cli
	./rvc_cli

rebuild: clean rvc_cli

clean:
	rm -f ./*.o
	rm -f ./rvc_cli