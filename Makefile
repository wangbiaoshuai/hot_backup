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
OBJS = .build/common_function.o .build/hot_backup_client.o .build/hot_backup_server.o .build/main.o

prefix:
	@mkdir -p .build

master: ${OBJS}
	${CXX} -o $@ $^ ${CXXFLAGS} -DSERVER

slave: ${OBJS}
	${CXX} -o $@ $^ ${CXXFLAGS} -DCLIENT

.build/common_function.o: src/common_function.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/hot_backup_client.o: src/hot_backup_client.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/hot_backup_server.o: src/hot_backup_server.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/main.o: src/main.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

package:
	cp master ./hot_backup_master/bin
	cp slave ./hot_backup_slave/bin

.PHONY: clean

clean:
	${RM} -rf .build master slave

