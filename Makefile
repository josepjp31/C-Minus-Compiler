# Makefile for C-Minus Compiler (Complete: Scanner, Parser & Semantic Analyzer)

CC = gcc
CFLAGS = -W -Wall -Isrc

SRCDIR = src

OBJS_SCANNER = main.o util.o scan.o
OBJS_PARSER  = main.o util.o lex.yy.o y.tab.o
OBJS_SEM     = main.o util.o lex.yy.o y.tab.o symtab.o analyze.o

.PHONY: all clean
all: cminus_semantic cminus_parser cminus_cimpl

# --- Ejecutables finales ---

cminus_semantic: $(OBJS_SEM)
	$(CC) $(CFLAGS) -g $(OBJS_SEM) -o $@ -lfl

cminus_parser: $(OBJS_PARSER)
	$(CC) $(CFLAGS) $(OBJS_PARSER) -o $@ -lfl

cminus_cimpl: $(OBJS_SCANNER)
	$(CC) $(CFLAGS) $(OBJS_SCANNER) -o $@

# --- Objetos C (origen en src/) ---

main.o: $(SRCDIR)/main.c $(SRCDIR)/globals.h $(SRCDIR)/util.h $(SRCDIR)/scan.h $(SRCDIR)/parse.h y.tab.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c

util.o: $(SRCDIR)/util.c $(SRCDIR)/util.h $(SRCDIR)/globals.h y.tab.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/util.c

scan.o: $(SRCDIR)/scan.c $(SRCDIR)/scan.h $(SRCDIR)/util.h $(SRCDIR)/globals.h y.tab.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/scan.c

symtab.o: $(SRCDIR)/symtab.c $(SRCDIR)/symtab.h $(SRCDIR)/globals.h
	$(CC) $(CFLAGS) -g -c $(SRCDIR)/symtab.c

analyze.o: $(SRCDIR)/analyze.c $(SRCDIR)/analyze.h $(SRCDIR)/globals.h $(SRCDIR)/symtab.h
	$(CC) $(CFLAGS) -g -c $(SRCDIR)/analyze.c

# --- Generación con Flex y Bison ---

lex.yy.o: lex.yy.c $(SRCDIR)/scan.h $(SRCDIR)/util.h $(SRCDIR)/globals.h y.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c

lex.yy.c: $(SRCDIR)/cminus.l y.tab.h
	flex $(SRCDIR)/cminus.l

y.tab.o: y.tab.c $(SRCDIR)/parse.h
	$(CC) $(CFLAGS) -c y.tab.c

y.tab.h: y.tab.c

y.tab.c: $(SRCDIR)/cminus.y
	yacc -d -v $(SRCDIR)/cminus.y

# --- Limpieza ---

clean:
	rm -vf cminus_semantic cminus_parser cminus_cimpl *.o lex.yy.c y.tab.c y.tab.h y.output