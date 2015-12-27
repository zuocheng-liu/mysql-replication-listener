<<<<<<< Updated upstream
#POCO=${HOME}/local/poco
=======
<<<<<<< HEAD
# POCO=${HOME}/local/poco
=======
#POCO=${HOME}/local/poco
>>>>>>> 2fb20c8887fd4cd14c74acdd064b8facb4ab71bd
>>>>>>> Stashed changes
CXX=g++
CXXLD=${CXX} -fPIC -shared -o 
BASE_DIR= .
BASE_INC= ${BASE_DIR}/include

INC= -I${BASE_INC} \
<<<<<<< Updated upstream

LIBDIR= 

LIB= 
=======
<<<<<<< HEAD
#	 -I${POCO}/include

#LIBDIR= -L${POCO}/lib

#LIB= -lm \
=======

LIBDIR= 

LIB= 
>>>>>>> 2fb20c8887fd4cd14c74acdd064b8facb4ab71bd
>>>>>>> Stashed changes

FLAG= ${LIBDIR} ${LIB} ${INC}


TARGET=lib/libmysqlreplication.so
SRCS=$(wildcard src/*.cpp)
SRCSNOTDIR=$(notdir $(SRCS))
OBJS=$(patsubst %.cpp, compile/%.o, $(SRCSNOTDIR)) 


# initial dir

$(shell if [ ! -d compile ]; then mkdir compile;  fi;)
$(shell if [ ! -d lib ]; then mkdir lib;  fi;)

.PHONY: clean test

$(TARGET):$(OBJS)
	${CXXLD} $@ $^ ${FLAG} 

compile/%.o:src/%.cpp
	${CXX} -fPIC -o $@ -c $< ${FLAG} 

clean :
	rm -rf $(TARGET) $(OBJS) 

#include testsuite/Makefile
