#OPTIONS=-D_TRACE_PARSER -D_EXT_PARSER
#OPTIONS=-D_EXT_PARSER
OPTIONS=-D_CLI_MAIN -D_TRACE_PARSER

CC=gcc $(OPTIONS)
LINK=ld
FLEX=flex
BISON=yacc

FTD2XX_H=ftd2xx.h
WIN_TYPES_H=WinTypes.h

INCLUDES=-I.
LINKS=-lftd2xx -lpthread
LINK_DIRS=-L.
FRAMEWORKS=-framework QuickTime -framework Cocoa

all:	parser

config_parser.y	\
config_reader.l	\
:	objs flex

bison: 
	$(BISON) -d -b config_parser config_parser.y

flex: bison
	$(FLEX) -oconfig_reader.yy.c config_reader.l

objs:	\
config_reader.yy.o	\
config_parser.tab.o	\
dmx_controller.o	\
flicker_effect.o	\
oscillator_effect.o	\
timed_effect.o	\
show_handler.o	\
sound_analyzer.o

parser: flex objs 
	$(CC) $(LINK_DIRS) $(LINKS) $(FRAMEWORKS) $(INCLUDES) -o parse show_handler.o dmx_controller.o \
flicker_effect.o sound_analyzer.o oscillator_effect.o timed_effect.o config_reader.yy.c config_parser.tab.c

config_reader.yy.o:	config_reader.yy.c
	$(CC) -c -o config_reader.yy.o $(INCLUDES) config_reader.yy.c

config_parser.tab.o:	config_parser.tab.c
	$(CC) -c -o config_parser.tab.o $(INCLUDES) config_parser.tab.c

dmx_controller.o:	dmx_controller.c
	$(CC) -c -o dmx_controller.o $(INCLUDES) dmx_controller.c 

flicker_effect.o:	flicker_effect.c
	$(CC) -c -o flicker_effect.o $(INCLUDES) flicker_effect.c

oscillator_effect.o:	oscillator_effect.c
	$(CC) -c -o oscillator_effect.o $(INCLUDES) oscillator_effect.c

timed_effect.o:	timed_effect.c
	$(CC) -c -o timed_effect.o $(INCLUDES) timed_effect.c

show_handler.o:	show_handler.c
	$(CC) -c -o show_handler.o $(INCLUDES) show_handler.c

sound_analyzer.o:	sound_analyzer.c
	$(CC) -c -o sound_analyzer.o $(INCLUDES) sound_analyzer.c

clean:
	-rm -rf *.o 2>/dev/null
	-rm -rf config_reader.yy.c
	-rm -rf config_parser.tab.c
	-rm -rf config_parser.tab.y
	-rm -rf parse
