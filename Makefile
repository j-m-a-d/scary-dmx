#OPTIONS=-D_TRACE_PARSER -D_EXT_PARSER -D_REENTRANT
OPTIONS=-D_CLI_MAIN -D_TRACE_PARSER -D_REENTRANT -m32

PROG=playshow
CC=gcc $(OPTIONS)
LINK=ld
FLEX=flex
BISON=yacc

FTD2XX_H=ftd2xx.h
WIN_TYPES_H=WinTypes.h

INCLUDES=-I.
LIBS=-lftd2xx -lpthread
LINK_DIRS=-L.
FRAMEWORKS=-framework QuickTime -framework Carbon -framework CoreFoundation

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
	$(CC) $(LINK_DIRS) $(LIBS) $(FRAMEWORKS) $(INCLUDES) -o $(PROG) config_reader.yy.o	\
config_parser.tab.o	\
dmx_controller.o	\
flicker_effect.o	\
oscillator_effect.o	\
timed_effect.o	\
show_handler.o	\
sound_analyzer.o


config_reader.yy.o:	config_reader.yy.c
	$(CC) -c -o config_reader.yy.o $(INCLUDES) config_reader.yy.c

config_parser.tab.o:	config_parser.tab.c
	$(CC) -c -o config_parser.tab.o $(INCLUDES) config_parser.tab.c

dmx_controller.o:	dmx_controller.c dmx_controller.h
	$(CC) -c -o dmx_controller.o $(INCLUDES) dmx_controller.c 

flicker_effect.o:	flicker_effect.c flicker_effect.h
	$(CC) -c -o flicker_effect.o $(INCLUDES) flicker_effect.c

oscillator_effect.o:	oscillator_effect.c oscillator_effect.h
	$(CC) -c -o oscillator_effect.o $(INCLUDES) oscillator_effect.c

timed_effect.o:	timed_effect.c timed_effect.h
	$(CC) -c -o timed_effect.o $(INCLUDES) timed_effect.c

show_handler.o:	show_handler.c show_handler.h
	$(CC) -c -o show_handler.o $(INCLUDES) show_handler.c

sound_analyzer.o:	sound_analyzer.c sound_analyzer.h
	$(CC) -c -o sound_analyzer.o  $(INCLUDES) sound_analyzer.c

clean:
	-rm -rf *.o 2>/dev/null
	-rm -rf config_reader.yy.c
	-rm -rf config_parser.tab.c
	-rm -rf config_parser.tab.y
	-rm -rf $(PROG)
