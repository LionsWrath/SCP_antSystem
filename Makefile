CC=gcc
ANTS=160
CYCLES=10000
THREADS=2

default:
	$(CC) -gstabs set_cover_scp.c -lm -o set_cover_scp.out
	$(CC) -gstabs set_cover_scp_parallel.c -lm -o set_cover_scp_parallel.out -lpthread

test:
	./set_cover_scp_parallel.out -a .25 -b 5 -r 0.5 -f Tests/test_01.dat -n ${ANTS} -c ${CYCLES} -t ${THREADS}
	./set_cover_scp.out -a .25 -b 5 -r 0.5 -f Tests/test_01.dat -n ${ANTS} -c ${CYCLES}
