all:
	make -C src

test: all
	./run_tests.pl

clean:
	make clean -C src
