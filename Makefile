# 
# ----------------------
# 
# Raul Sofia 2019225303
# Luís Miguel Gonçalves Leite 2021199102
# 
# Sistemas Operativos 2021/2022
# 
# ----------------------
# 

CC = gcc
OBJS = system_manager.o sensor.o user_console.o shared_memory.o
PROG = prog


all: $(PROG)

clean:
			rm ${OBJS} *~ ${PROG}

${PROG}:	${OBJS}
			${CC} ${OBJS} -o $@

.c.o:	
			${CC} $< -c -o $@

##########################################

system_manager.o: system_manager.c Shared_Memory.h
Sensor.o: Sensor.c
User_console.o: User_console.c
Shared_Memory.o: Shared_Memory.c Shared_Memory.h