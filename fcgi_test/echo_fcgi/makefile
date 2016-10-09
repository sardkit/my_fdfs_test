
LIBS+=-lfdfsclient -lfcgi -lhiredis -L/usr/local/lib

all: echo.cgi

#hello.cgi:
#	gcc fcgi_hello.c -lfcgi -o hello.cgi

echo.cgi: echo.c handleData.c upload_op.c make_log.c redis_op.c md5.c
	gcc $^ -I/usr/include/fastcommon -I/usr/include/fastdfs -I/usr/local/include/hiredis $(LIBS) -o echo.cgi

clean:
	rm -f *.o *.cgi
