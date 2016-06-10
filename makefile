CXX = clang++ -std=c++11
CXXFLAGS =
CXXFLAGS += -O0
CXXFLAGS += -Wall
CXXFLAGS += -Wextra

EXE = prefix.exe

all:	${EXE} simple bench
	./${EXE}


valgrind:	${EXE}
	valgrind ./${EXE}

prefix.o:	prefix.cpp prefix.hpp string.hpp
	${CXX} ${CXXFLAGS} -c $< -o $@

string.o:	string.cpp string.hpp
	${CXX} ${CXXFLAGS} -c $< -o $@

example.o:	example.cpp prefix.hpp
	${CXX} ${CXXFLAGS} -c $< -o $@

explicit_simple.ll:	explicit_simple.cpp prefix.hpp
	${CXX} ${CXXFLAGS} -c -S -emit-llvm $< -o $@
explicit_simple.s:	explicit_simple.cpp prefix.hpp
	${CXX} ${CXXFLAGS} -c -S $< -o $@

implicit_simple.ll:	implicit_simple.cpp prefix.hpp
	${CXX} ${CXXFLAGS} -c -S -emit-llvm $< -o $@
implicit_simple.s:	implicit_simple.cpp prefix.hpp
	${CXX} ${CXXFLAGS} -c -S $< -o $@

.PHONY:	simple
simple:	explicit_simple.ll explicit_simple.s implicit_simple.ll implicit_simple.s

catch.o:	catch.cpp catch.hpp
	${CXX} ${CXXFLAGS} -c -O3 $< -o $@

bench.cpp:	bench.py
	python3 $< > $@

check_bench.exe:	bench.cpp bench_main.cpp
	${CXX} ${CXXFLAGS} -DPRE=1 -DSTL=1 $^ -o $@

pre_bench.o:	bench.cpp
	${CXX} ${CXXFLAGS} -c -DPRE=1 $^ -o $@

pre_pass_bench.exe:	pre_bench.o bench_main.cpp
	${CXX} ${CXXFLAGS} -DPRE=1 $^ -o $@

pre_fail_bench.exe:	pre_bench.o bench_main.cpp
	${CXX} ${CXXFLAGS} -DFAILING=1 -DPRE=1 $^ -o $@

stl_bench.o:	bench.cpp
	${CXX} ${CXXFLAGS} -c -DSTL=1 $^ -o $@

stl_pass_bench.exe:	stl_bench.o bench_main.cpp
	${CXX} ${CXXFLAGS} -DSTL=1 $^ -o $@

stl_fail_bench.exe:	stl_bench.o bench_main.cpp
	${CXX} ${CXXFLAGS} -DFAILING=1 -DSTL=1 $^ -o $@

.PHONY:	bench
bench:	check_bench.exe stl_pass_bench.exe stl_fail_bench.exe pre_pass_bench.exe pre_fail_bench.exe
	./check_bench.exe


${EXE}:	prefix.o string.o example.o catch.o
	${CXX} ${CXXFLAGS} $^ -o $@

clean:
	rm -f *.o *.exe
	rm -f *.s *.ll
	rm -f bench.cpp
