
LIBS+=-lfcgi -lm -lhiredis -L/usr/local/lib

all: data.cgi

#hello.cgi:
#	gcc fcgi_hello.c -lfcgi -o hello.cgi

data.cgi: data.c handleData.c make_log.c redis_op.c cJSON.c
	gcc $^ -I/usr/local/include/hiredis $(LIBS) -o data.cgi

clean:
	rm -f *.o *.cgi
