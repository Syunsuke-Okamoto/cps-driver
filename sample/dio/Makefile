#CC = gcc
CC = arm-linux-gnueabi-gcc
LD = arm-linux-gnueabi-ld

INC = -I../../include/
BAT = -DCONFIG_DIO_BL

LIBCPSNAME=CpsDio

SRC_NAME001=dio
SRC_NAME002=dio_int_test
SRC_NAME003=dio_test
SRC_NAME004=dio_loop
SRC_NAME005=queryDeviceName

BIN_NAME001=${SRC_NAME001}
BIN_NAME002=${SRC_NAME001}_bl
BIN_NAME003=${SRC_NAME002}
BIN_NAME004=${SRC_NAME002}_bl
BIN_NAME005=${SRC_NAME003}
BIN_NAME006=${SRC_NAME003}_bl
BIN_NAME007=${SRC_NAME004}
BIN_NAME008=${SRC_NAME004}_bl
BIN_NAME009=${SRC_NAME005}

all:${BIN_NAME001}.o ${BIN_NAME002}.o ${BIN_NAME003}.o ${BIN_NAME004}.o ${BIN_NAME005}.o ${BIN_NAME006}.o ${BIN_NAME007}.o ${BIN_NAME008}.o ${BIN_NAME009}.o

${BIN_NAME001}.o:	
	${CC} ${INC} -o ${BIN_NAME001} ${SRC_NAME001}.c -l$(LIBCPSNAME)

${BIN_NAME002}.o:
	${CC} ${INC} -o ${BIN_NAME002} ${BAT} ${SRC_NAME001}.c -l$(LIBCPSNAME)

${BIN_NAME003}.o:
	${CC} ${INC} -o ${BIN_NAME003} ${SRC_NAME002}.c -l$(LIBCPSNAME)

${BIN_NAME004}.o:
	${CC} ${INC} -o ${BIN_NAME004} ${BAT} ${SRC_NAME002}.c -l$(LIBCPSNAME)

${BIN_NAME005}.o:
	${CC} ${INC} -o ${BIN_NAME005} ${SRC_NAME003}.c -l$(LIBCPSNAME)

${BIN_NAME006}.o:
	${CC} ${INC} -o ${BIN_NAME006} ${BAT} ${SRC_NAME003}.c -l$(LIBCPSNAME)

${BIN_NAME007}.o:
	${CC} ${INC} -o ${BIN_NAME007} ${SRC_NAME004}.c -l$(LIBCPSNAME)

${BIN_NAME008}.o:
	${CC} ${INC} -o ${BIN_NAME008} ${BAT} ${SRC_NAME004}.c -l$(LIBCPSNAME)

${BIN_NAME009}.o:
	${CC} ${INC} -o ${BIN_NAME009} ${SRC_NAME005}.c -l$(LIBCPSNAME)

clean:
	rm -f ${BIN_NAME001} ${BIN_NAME002} ${BIN_NAME003} ${BIN_NAME004} ${BIN_NAME005} ${BIN_NAME006} ${BIN_NAME007} ${BIN_NAME008} ${BIN_NAME009}
