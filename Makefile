# Makefile for C-Minus Compiler (Scanner, Parser & Semantic Analyzer)

CC = gcc
CFLAGS = -W -Wall -I. -Isrc

SRCDIR = src

OBJS_SCANNER = main_lex.o util_lex.o scan.o
OBJS_PARSER  = main_parse.o util.o lex.yy.o y.tab.o
OBJS_SEM     = main.o util.o lex.yy.o y.tab.o symtab.o analyze.o

.PHONY: all clean
all: cminus_semantic cminus_parser cminus_cimpl

# --- Executables ---

cminus_semantic: $(OBJS_SEM)
	$(CC) $(CFLAGS) -g $(OBJS_SEM) -o $@ -lfl

cminus_parser: $(OBJS_PARSER)
	$(CC) $(CFLAGS) $(OBJS_PARSER) -o $@ -lfl

cminus_cimpl: $(OBJS_SCANNER)
	$(CC) $(CFLAGS) $(OBJS_SCANNER) -o $@

# --- C Objects ---

main.o: $(SRCDIR)/main.c $(SRCDIR)/globals.h $(SRCDIR)/util.h $(SRCDIR)/scan.h $(SRCDIR)/parse.h y.tab.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $@

main_parse.o: $(SRCDIR)/main.c $(SRCDIR)/globals.h $(SRCDIR)/util.h $(SRCDIR)/scan.h $(SRCDIR)/parse.h y.tab.h
	$(CC) $(CFLAGS) -DNO_ANALYZE=TRUE -c $(SRCDIR)/main.c -o $@

main_lex.o: $(SRCDIR)/main.c $(SRCDIR)/globals.h $(SRCDIR)/util.h $(SRCDIR)/scan.h
	$(CC) $(CFLAGS) -DNO_PARSE=TRUE -c $(SRCDIR)/main.c -o $@

util.o: $(SRCDIR)/util.c $(SRCDIR)/util.h $(SRCDIR)/globals.h y.tab.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/util.c -o $@

util_lex.o: $(SRCDIR)/util.c $(SRCDIR)/util.h $(SRCDIR)/globals.h
	$(CC) $(CFLAGS) -DNO_PARSE=TRUE -c $(SRCDIR)/util.c -o $@

scan.o: $(SRCDIR)/scan.c $(SRCDIR)/scan.h $(SRCDIR)/util.h $(SRCDIR)/globals.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/scan.c -o $@

symtab.o: $(SRCDIR)/symtab.c $(SRCDIR)/symtab.h $(SRCDIR)/globals.h
	$(CC) $(CFLAGS) -g -c $(SRCDIR)/symtab.c -o $@

analyze.o: $(SRCDIR)/analyze.c $(SRCDIR)/analyze.h $(SRCDIR)/globals.h $(SRCDIR)/symtab.h
	$(CC) $(CFLAGS) -g -c $(SRCDIR)/analyze.c -o $@

# --- Flex and Bison ---

lex.yy.o: lex.yy.c $(SRCDIR)/scan.h $(SRCDIR)/util.h $(SRCDIR)/globals.h y.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c -o $@

lex.yy.c: $(SRCDIR)/cminus.l y.tab.h
	flex $(SRCDIR)/cminus.l

y.tab.o: y.tab.c $(SRCDIR)/parse.h
	$(CC) $(CFLAGS) -c y.tab.c -o $@

y.tab.h: y.tab.c

y.tab.c: $(SRCDIR)/cminus.y
	yacc -d -v $(SRCDIR)/cminus.y

# --- Cleanup ---

clean:
	rm -vf cminus_semantic cminus_parser cminus_cimpl *.o lex.yy.c y.tab.c y.tab.h y.output