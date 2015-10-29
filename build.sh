gcc -w -fPIC -DMI_SERVBUILD -I$INFORMIXDIR/incl/public  -I$INFORMIXDIR/incl -L$INFORMIXDIR/esql/lib -c informix_socket_streaming.c

gcc informix_socket_streaming.o -shared -o informix_socket_streaming.so
