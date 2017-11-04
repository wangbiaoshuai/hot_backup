all: hot_backup_server

ifeq ($(debug), 1)
    DEBUG = -g
else
    DEBUG = -O2
endif

CXX = g++
LIB = -Wl,-R,"./" \
	  -L./log4cplus/lib -llog4cplus -lrt \
	  -L/usr/lib64/mysql -lmysqlclient
INCLUDE = -I./log4cplus/include
CXXFLAGS = ${INCLUDE} ${LIB} -pthread -DREENTRANT ${DEBUG}

hot_backup_server: main.o hot_backup_server.o common_function.o
	${CXX} -o ./bin/$@ $^ ${CXXFLAGS}

main.o: main.cc
	${CXX} -c $^ ${CXXFLAGS}

hot_backup_server.o: hot_backup_server.cc
	${CXX} -c $^ ${CXXFLAGS}

common_function.o: common_function.cc
	${CXX} -c $^ ${CXXFLAGS}


.PHONY: clean

clean:
	${RM} *.o ./bin/hot_backup_server
