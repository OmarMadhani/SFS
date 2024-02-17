# Name: Omar Madhani
# V00978484

all: diskinfo disklist diskget diskput

diskinfo: diskinfo.o
	gcc diskinfo.o -o diskinfo

diskinfo.o: diskinfo.c diskfunctions.h
	gcc -c diskinfo.c

disklist: disklist.o
	gcc disklist.o -o disklist

disklist.o: disklist.c diskfunctions.h
	gcc -c disklist.c

diskget: diskget.o
	gcc diskget.o -o diskget

diskget.o: diskget.c diskfunctions.h
	gcc -c diskget.c

diskput: diskput.o
	gcc diskput.o -o diskput

diskput.o: diskput.c diskfunctions.h
	gcc -c diskput.c

clean:
	rm -rf *.o diskinfo disklist diskget diskput
