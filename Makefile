OPTIONS=-D_TRACE_PARSER -D_LOG -D_REENTRANT -m32 -Wall

CC=clang
LINK=ld
FLEX=flex
BISON=yacc

INCLUDES=-I.
LIBS=-L. -lftd2xx
FRAMEWORKS=-framework CoreServices -framework QuickTime 

# TODO fix path
CORE_DIR=-L /Users/jason/Library/Developer/Xcode/DerivedData/Scary_DMX-guwojqbpiucwrgdzlxnicqsgdmrg/Build/Products/Debug/ -lscarydmx-core


all:	config_parser.o

config_parser.tab.c:	config_parser.y
	$(BISON) -d -b config_parser config_parser.y

config_reader.yy.c: config_parser.tab.c config_reader.l
	$(FLEX) -oconfig_reader.yy.c config_reader.l

config_reader.yy.o: config_reader.yy.c
	$(CC) $(OPTIONS) $(INCLUDES) -c config_reader.yy.c

config_parser.o: config_reader.yy.o config_parser.tab.c
	$(CC) $(OPTIONS) $(INCLUDES) -c -o config_parser.o config_reader.yy.o config_parser.tab.c

parser_test:	parser_test.c
	$(CC) $(OPTIONS) $(INCLUDE) -o parser_test $< $(CORE_DIR) $(LIBS) $(FRAMEWORKS)

chaser_test:	chaser_test.c
	$(CC) $(OPTIONS) $(INCLUDE) -o chaser_test $< $(CORE_DIR) $(LIBS) $(FRAMEWORKS)

clean:
	-rm -rf *.o 2>/dev/null
	-rm -rf parser_test
	-rm -rf chaser_test
	-rm -rf config_reader.yy.c
	-rm -rf config_parser.tab.c
	-rm -rf config_parser.tab.y
	-rm -rf config_parser.tab.h

