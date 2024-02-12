articleml: articleml.c m_leg.c
	cc -g -I/usr/include/libxml2 -c articleml.c -o articleml.o
	cc -g -o articleml articleml.o -lxml2

m_leg.c: m.leg m.h
	leg -o m_leg.c m.leg

clean:
	rm articleml.o m_leg.c

install: articleml
	cp articleml /usr/local/bin/

documentation.html: doc/documentation.xml articleml
	cat doc/documentation.xml | ./articleml > doc/documentation.html

test_out.html: example/example.xml
	cat example/example.xml | ./articleml > example/test_out.html

