CC_EXEC=g++
CC_FLAGS=-std=c++11

CC= ${CC_EXEC} ${CC_FLAGS}

BUILD_DIR=build

executable: ${BUILD_DIR}/main.o ${BUILD_DIR}/Filters.o
	${CC_EXEC} ${BUILD_DIR}/main.o ${BUILD_DIR}/Filters.o -o ImageFilters.out

${BUILD_DIR}/main.o: main.cpp Filters.hpp
	${CC} -c main.cpp -o ${BUILD_DIR}/main.o

${BUILD_DIR}/Filters.o: Filters.cpp Filters.hpp
	${CC} -c Filters.cpp -o ${BUILD_DIR}/Filters.o

clean:
	rm -rf ${BUILD_DIR} && mkdir -p ${BUILD_DIR}