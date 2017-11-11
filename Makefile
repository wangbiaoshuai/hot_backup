all: prefix master slave

ifeq (${debug}, 1)
    DEBUG = -g
else
    DEBUG = -O2
endif

CXX = g++
INCLUDE = -I./src/log4cplus/include
LIBS = -Wl,-R,"./" \
	   -L./src/log4cplus/lib -llog4cplus -lrt \
	   -L./libs -lmysqlclient
CXXFLAGS = -Wall ${INCLUDE} ${LIBS} ${DEBUG}
OBJS = .build/common_function.o .build/hot_backup_client.o .build/hot_backup_server.o

prefix:
	@mkdir -p .build

master: ${OBJS} .build/main_master.o
	${CXX} -o $@ $^ ${CXXFLAGS}

slave: ${OBJS} .build/main_slave.o
	${CXX} -o $@ $^ ${CXXFLAGS}

.build/common_function.o: src/common_function.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/hot_backup_client.o: src/hot_backup_client.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/hot_backup_server.o: src/hot_backup_server.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/main_master.o: src/main.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS} -DSERVER

.build/main_slave.o: src/main.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS} -DCLIENT

package:
	cp master ./hot_backup_master/bin/hot_backup_master
	cp slave ./hot_backup_slave/bin/hot_backup_slave

.PHONY: clean

clean:
	${RM} -rf .build master slave

