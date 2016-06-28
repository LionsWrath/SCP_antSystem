CC=gcc
ANTS=100
CYCLES=5000
THREADS=2
SEED=123456789

default:
	$(CC) -gstabs set_cover_scp.c -lm -o set_cover_scp.out
#	$(CC) -gstabs set_cover_scp_parallel.c -lm -o set_cover_scp_parallel.out -lpthread
	$(CC) -gstabs set_cover_scp_more_parallel.c -lm -o set_cover_scp_more_parallel.out -lpthread

test:
	./set_cover_scp_more_parallel.out -a .25 -b 5 -r 0.5 -f Tests/test_01.dat -n ${ANTS} -c ${CYCLES} -t 1 -s ${SEED}
	./set_cover_scp_more_parallel.out -a .25 -b 5 -r 0.5 -f Tests/test_01.dat -n ${ANTS} -c ${CYCLES} -t ${THREADS} -s ${SEED}
#	./set_cover_scp_parallel.out -a .25 -b 5 -r 0.5 -f Tests/test_01.dat -n ${ANTS} -c ${CYCLES} -t ${THREADS}
	./set_cover_scp.out -a .25 -b 5 -r 0.5 -f Tests/test_01.dat -n ${ANTS} -c ${CYCLES} -s ${SEED}

