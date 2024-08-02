CC_EXEC=gcc

CC=${CC_EXEC}

BUILD_DIR=build
CJSON_DIR=cJSON-1.7.18

all: Restaurant Customer Supplier JSONParser

${BUILD_DIR}/restaurant.o: restaurant.c UDP.h TCP.h ${CJSON_DIR}/cJSON.h
	${CC} -c restaurant.c -o ${BUILD_DIR}/restaurant.o -I${CJSON_DIR}

${BUILD_DIR}/customer.o: customer.c UDP.h TCP.h ${CJSON_DIR}/cJSON.h
	${CC} -c customer.c -o ${BUILD_DIR}/customer.o -I${CJSON_DIR}

${BUILD_DIR}/supplier.o: supplier.c UDP.h TCP.h ${CJSON_DIR}/cJSON.h
	${CC} -c supplier.c -o ${BUILD_DIR}/supplier.o -I${CJSON_DIR}

${BUILD_DIR}/UDP.o: UDP.c UDP.h
	${CC} -c UDP.c -o ${BUILD_DIR}/UDP.o

${BUILD_DIR}/TCP.o: TCP.c TCP.h
	${CC} -c TCP.c -o ${BUILD_DIR}/TCP.o

${BUILD_DIR}/cJSON.o: ${CJSON_DIR}/cJSON.c ${CJSON_DIR}/cJSON.h
	${CC} -c ${CJSON_DIR}/cJSON.c -o ${BUILD_DIR}/cJSON.o -I${CJSON_DIR}

Restaurant: ${BUILD_DIR}/restaurant.o ${BUILD_DIR}/UDP.o ${BUILD_DIR}/TCP.o ${BUILD_DIR}/cJSON.o
	${CC_EXEC} ${BUILD_DIR}/restaurant.o ${BUILD_DIR}/UDP.o ${BUILD_DIR}/TCP.o ${BUILD_DIR}/cJSON.o -o restaurant.out

Customer: ${BUILD_DIR}/customer.o ${BUILD_DIR}/UDP.o ${BUILD_DIR}/TCP.o ${BUILD_DIR}/cJSON.o
	${CC_EXEC} ${BUILD_DIR}/customer.o ${BUILD_DIR}/UDP.o ${BUILD_DIR}/TCP.o ${BUILD_DIR}/cJSON.o -o customer.out

Supplier: ${BUILD_DIR}/supplier.o ${BUILD_DIR}/UDP.o ${BUILD_DIR}/TCP.o ${BUILD_DIR}/cJSON.o
	${CC_EXEC} ${BUILD_DIR}/supplier.o ${BUILD_DIR}/UDP.o ${BUILD_DIR}/TCP.o ${BUILD_DIR}/cJSON.o -o supplier.out

JSONParser: ${BUILD_DIR}/json_parser.o ${BUILD_DIR}/cJSON.o
	${CC_EXEC} ${BUILD_DIR}/json_parser.o ${BUILD_DIR}/cJSON.o -o json_parser.out

clean:
	rm -rf ${BUILD_DIR} && mkdir -p ${BUILD_DIR}
	rm -f *.out
