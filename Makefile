
PREFIX=/usr/local

all: articleml.so articleml

articleml.so: articleml.o 
	cc -fPIC -shared -o libarticleml.so articleml.o -lxml2

articleml.o: articleml.c m_leg.c
	cc -g -fPIC -I/usr/include/libxml2 -c articleml.c -o articleml.o

articleml: articleml.o main.c
	cc -I/usr/include/libxml2 -c main.c -o main.o
	cc -g -o articleml main.o articleml.o -lxml2

m_leg.c: m.leg m.h
	leg -o m_leg.c m.leg

clean:
	rm articleml.o
	rm m_leg.c
	rm main.o

install: articleml
	install -m 0755 articleml $(PREFIX)/bin
	install -m 0755 libarticleml.so $(PREFIX)/lib
	install -m 0644 articleml.h $(PREFIX)/include

documentation.html: docs/documentation.xml articleml
	cat docs/documentation.xml | ./articleml > docs/documentation.html

test_out.html: example/example.xml
	cat example/example.xml | ./articleml > example/test_out.html

