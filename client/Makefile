all: hot_backup_client

ifeq (${debug}, 1)
    DEBUG = -g
else
    DEBUG = -O2
endif

CXX = g++
INCLUDE = -I./log4cplus/include
LIB = -L./log4cplus/lib -llog4cplus
CXXFLAGS = ${INCLUDE} ${LIB} -lrt ${DEBUG}

hot_backup_client: main.o hot_backup_client.o common_function.o
	${CXX} -o ./bin/$@ $^ ${CXXFLAGS}

main.o: main.cc
	${CXX} -c $^ ${CXXFLAGS}

hot_backup_client.o: hot_backup_client.cc
	${CXX} -c $^ ${CXXFLAGS}

common_function.o: common_function.cc
	${CXX} -c $^ ${CXXFLAGS}


.PHONY: clean

clean:
	${RM} ./bin/hot_backup_client *.o

