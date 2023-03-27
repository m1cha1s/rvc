CC := gcc
CFLAGS := -Wall -std=c99 -pedantic

rvc_cli: main.o
	$(CC) $^ -o $@

%.c: %.o
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: run run_tests clean clean_test_progs clean_all rebuild test_progs watch

test_progs:
	make -C test_progs all

run: rvc_cli test_progs
	./rvc_cli

run_tests: rvc_cli test_progs
	./rvc_cli test_progs/add-addi.bin

rebuild: clean rvc_cli

watch:
	fswatch -0 -v -o ./rvc.h -o ./main.c | xargs -0 -n 1 -I {} make rebuild

clean:
	rm -f ./*.o
	rm -f ./rvc_cli

clean_test_progs:
	make -C test_progs clean

clean_all: clean clean_test_progs

commit:
	git add . && \
	git commit && \
	git push