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
OBJS = system_manager.o sensor.o user_console.o shared_memory.o log.o
PROG = home_iot
FLAGS = -Wall -pthread


all: $(PROG)

clean:
			rm ${OBJS} *~ ${PROG}

${PROG}:	${OBJS}
			${CC} ${OBJS} ${FLAGS} -o $@

.c.o:	
			${CC} ${FLAGS} $< -c -o $@

##########################################

system_manager.o: system_manager.c log.h shared_memory.h
sensor.o: Sensor.c
user_console.o: user_console.c
shared_memory.o: shared_memory.c shared_memory.h
log.o: log.c log.h
