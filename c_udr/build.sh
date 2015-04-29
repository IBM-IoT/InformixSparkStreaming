gcc -fPIC -DMI_SERVBUILD -I$INFORMIXDIR/incl/public  -I$INFORMIXDIR/incl -L$INFORMIXDIR/esql/lib -c c_udr.c

gcc c_udr.o -shared -o c_udr.so

cp c_udr.so /opt/informix/extend/c_udr.so

ls -l /opt/informix/extend/c_udr.so