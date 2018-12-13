RM        := rm -rf
MAIN      := ./push
C++       := g++
CFLAGS    := -W -Wall -std=c++11 -pthread -lpigpio -lrt
INCLUDES	:= -I./cpp/
DEPEND		:= ./cpp/3dPrinterOs.h ./cpp/Object.h
FILES     := ./cpp/main.cpp

all: ${DEPEND}
	${C++} ${CFLAGS} ${FILES} ${INCLUDES} -o ${MAIN}

clean:
	$(RM) ${MAIN}
