CC=clang
CC_OPTS=-D_TRACE_PARSER -D_LOG -m32 -Wall

LINK=ld
FLEX=flex
BISON=yacc
BISON_OPTS=-Wall -d

INCLUDES=-I.
LIBS=



all:	parser

bison: 
	$(BISON) $(BISON_OPTS) -b config_parser config_parser.y

flex: bison
	$(FLEX) -oconfig_reader.yy.c config_reader.l

parser: flex
	$(CC) $(OPTIONS) $(INCLUDES) -c config_reader.yy.c 
	$(CC) $(OPTIONS) $(INCLUDES) -c -o config_parser.o config_reader.yy.o config_parser.tab.c

clean:
	-rm -rf *.o 2>/dev/null
	-rm -rf config_reader.yy.c
	-rm -rf config_parser.tab.c
	-rm -rf config_parser.tab.y
	-rm -rf config_parser.tab.h
