# 
# ----------------------
# 
# Luís Miguel Gonçalves Leite 2021199102
# 
# Sistemas Operativos 2022/2023
# 
# ----------------------
# 

CC = gcc
OBJS = system_manager.o shared_memory.o log.o
OBJS2 = sensor.o shared_memory.o log.o
OBJS3 = user_console.o
PROG = home_iot
SENSOR = sensor
FLAGS = -Wall -pthread
USER_CONSOLE = user_console

all: $(PROG) ${SENSOR} ${USER_CONSOLE}

clean:
			rm ${OBJS} ${OBJS2} ${OBJS3} *~ ${PROG} ${SENSOR} ${USER_CONSOLE} log.txt

${PROG}:	${OBJS}
			${CC} ${OBJS} ${FLAGS} -o $@

${SENSOR}:	${OBJS2}
			${CC} ${OBJS2} ${FLAGS} -o $@

${USER_CONSOLE}:	${OBJS3}
					${CC} ${OBJS3} ${FLAGS} -o $@

.c.o:	
			${CC} ${FLAGS} $< -c -o $@

##########################################

system_manager.o: system_manager.c log.h Shared_Memory.h
shared_memory.o: shared_memory.c Shared_Memory.h
log.o: log.c log.h
