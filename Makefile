CC = gcc
TARGET = packcap
OBJS = src/packcap.o 


$(TARGET) : $(OBJS)
	$(CC) $^ -o $@

clean:
	rm -f packcap
	rm -f src/packcap.o