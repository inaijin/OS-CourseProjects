CXX_EXEC=g++

CXX= ${CXX_EXEC}

CXXFLAGS = -std=c++17 -Wall -Wextra

BUILD_DIR=build

all: Main Houses Contor Facility

${BUILD_DIR}/mainProcess.o: mainProcess.cpp
	${CXX} ${CXXFLAGS} -c mainProcess.cpp -o ${BUILD_DIR}/mainProcess.o

${BUILD_DIR}/houses.o: houses.cpp
	${CXX} ${CXXFLAGS} -c houses.cpp -o ${BUILD_DIR}/houses.o

${BUILD_DIR}/contors.o: contors.cpp
	${CXX} ${CXXFLAGS} -c contors.cpp -o ${BUILD_DIR}/contors.o

${BUILD_DIR}/facility.o: facility.cpp
	${CXX} ${CXXFLAGS} -c facility.cpp -o ${BUILD_DIR}/facility.o

Main: ${BUILD_DIR}/mainProcess.o
	${CXX_EXEC} ${BUILD_DIR}/mainProcess.o -o mainProcess.out

Houses: ${BUILD_DIR}/houses.o
	${CXX_EXEC} ${BUILD_DIR}/houses.o -o houses.out

Contor: ${BUILD_DIR}/contors.o
	${CXX_EXEC} ${BUILD_DIR}/contors.o -o contors.out

Facility: ${BUILD_DIR}/facility.o
	${CXX_EXEC} ${BUILD_DIR}/facility.o -o facility.out

clean:
	rm -rf ${BUILD_DIR} && mkdir -p ${BUILD_DIR}