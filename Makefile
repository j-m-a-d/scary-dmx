OPTIONS=-D_TRACE_PARSER -D_LOG -D_REENTRANT -m32 -Wall

CC=clang
LINK=ld
FLEX=flex
BISON=yacc

INCLUDES=-I.
LIBS=


all:	config_parser.o

config_parser.tab.c:	config_parser.y
	$(BISON) -d -b config_parser config_parser.y

config_reader.yy.c: config_parser.tab.c config_reader.l
	$(FLEX) -oconfig_reader.yy.c config_reader.l

config_reader.yy.o: config_reader.yy.c
	$(CC) $(OPTIONS) $(INCLUDES) -c config_reader.yy.c

config_parser.o: config_reader.yy.o config_parser.tab.c
	$(CC) $(OPTIONS) $(INCLUDES) -c -o config_parser.o config_reader.yy.o config_parser.tab.c

clean:
	-rm -rf *.o 2>/dev/null
	-rm -rf config_reader.yy.c
	-rm -rf config_parser.tab.c
	-rm -rf config_parser.tab.y
	-rm -rf config_parser.tab.h

