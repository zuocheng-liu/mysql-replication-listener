MYSQLRL=${HOME}/local/mysql-replication-listener
CXX=g++
CXXLD=${CXX} -fPIC -shared -o 

BASE_DIR= .
BASE_INC= ${BASE_DIR}/include

INC= -I${BASE_INC} \
	 -I${MYSQLRL}/include

LIB= -lmysqlreplication

LIBDIR= -L${MYSQLRL}/lib

FLAG= ${LIBDIR} ${LIB} ${INC}

#TARGET=example1
SRCS=$(wildcard src/*.cpp)
SRCSNOTDIR=$(notdir $(SRCS))
OBJS=$(patsubst %.cpp, compile/%.o, $(SRCSNOTDIR)) 


# initial dir

$(shell if [ ! -d compile ]; then mkdir compile;  fi;)
$(shell if [ ! -d lib ]; then mkdir lib;  fi;)

.PHONY: clean test


#$(TARGET):$(OBJS)
#	${CXX} $@ $^ ${FLAG} 

compile/%.o:src/%.cpp
	${CXX} -o $@ -c $< ${FLAG} 

compile/%.o:%.cpp
	${CXX} -o $@ -c $< ${FLAG} 


example1: compile/basic-1.o
	${CXX} -o $@ $^ ${FLAG} 


clean :
	rm -rf $(TARGET) $(OBJS) example1

#include testsuite/Makefile
