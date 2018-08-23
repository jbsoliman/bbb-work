#Simple makefile for building basic application

TARGET = as2
SOURCES =  segDisplay.c sorter.c a2d.c main.c udpclient.c

TARGET2 = noworky
SOURCES2 = noworky.c
 
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILE)gcc 
CFLAGS =  -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -pthread 

default: app done

as2: app done

help:
	@echo "Targets include all, app, and clean."

all: app appNoworky doneAll
app:
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGET) -lm

appNoworky:
	$(CC_C) $(CFLAGS) $(SOURCES2) -o $(OUTDIR)/$(TARGET2) 

done:
	@echo "Finished building application."

doneAll:
	@echo "Finished building noworky & sorter application."

noworky: 
	$(CC_C) $(CFLAGS) $(SOURCES2) -o $(OUTDIR)/$(TARGET2)

clean:
	rm $(OUTDIR)/$(TARGET)
