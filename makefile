# Makefile for CSCB09 A3 - System Monitoring Tool: Concurrency and Signals


CC = gcc

OBJECTS = systemMonitoringTool.o getStats.o

main: systemMonitoringTool

systemMonitoringTool: $(OBJECTS)
	${CC} systemMonitoringTool.o getStats.o -o systemMonitoringTool

systemMonitoringTool.o: systemMonitoringTool.c
	${CC} -c systemMonitoringTool.c -o systemMonitoringTool.o

getStats.o: getStats.c
	${CC} -c getStats.c -o getStats.o

.PHONY: clean

clean:
	rm -f *.o systemMonitoringTool
