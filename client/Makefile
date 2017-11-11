all: hot_backup_client

ifeq (${debug}, 1)
    DEBUG = -g
else
    DEBUG = -O2
endif

CXX = g++
INCLUDE = -I./log4cplus/include
LIB = -Wl,-R,"./" \
	  -L./log4cplus/lib -llog4cplus -lrt \
	  -L/usr/lib64/mysql -lmysqlclient \
	  -pthread
CXXFLAGS = -Wall ${INCLUDE} ${LIB} -lrt ${DEBUG}

hot_backup_client: main.o hot_backup_client.o common_function.o hot_backup_server.o
	${CXX} -o ./bin/$@ $^ ${CXXFLAGS}

main.o: main.cc
	${CXX} -c $^ ${CXXFLAGS}

hot_backup_client.o: hot_backup_client.cc
	${CXX} -c $^ ${CXXFLAGS}

common_function.o: common_function.cc
	${CXX} -c $^ ${CXXFLAGS}

hot_backup_server.o: hot_backup_server.cc
	${CXX} -c $^ ${CXXFLAGS}


.PHONY: clean

clean:
	${RM} ./bin/hot_backup_client *.o
	rm -rf hot_backup_client

package:
	mkdir -p hot_backup_client
	cp -r ./bin ./hot_backup_client
	mv ./hot_backup_client/bin/install.sh ./hot_backup_client
	cp ./hot_backup_client/bin/uninstall.sh ./hot_backup_client

