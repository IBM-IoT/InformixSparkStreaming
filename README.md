# Informix Socket Streaming

A feature for the Informix database that enables us to stream data that was added to a database through ```INSERT``` 
and ```UPDATE```(TODO) statements through a TCP socket.

That streaming data can then be consumed by any program that can use sockets as a data source. This is useful for 
streaming data in real-time to an analytics platform such as Apache Spark.

## Build

In the same directory as the ```informix_socket_streaming.c``` file, run the following commands. ```$INFORMIXDIR``` should 
be the root folder of your Informix installation.

```gcc -w -fPIC -DMI_SERVBUILD -I$INFORMIXDIR/incl/public  -I$INFORMIXDIR/incl -L$INFORMIXDIR/esql/lib -c informix_socket_streaming.c```

```gcc informix_socket_streaming.o -shared -o informix_socket_streaming.so```

The end result of these commands will be the ```informix_socket_streaming.so``` file that will be used in the following 
installation step.

## INSTALL

Take the ```informix_socket_streaming.so``` that was created in the previous build step and copy it to the $INFORMIXDIR/extend/
directory.

This shared object file contains all the code needed to create the UDRs that will be used in the creation of the 
secondary access method.

Now run ```create_socket_streaming_virtual_index.sql``` and you will create a secondary access method called
```informix_socket_streaming```. This is the access method you will use when creating a socket streaming index.

## USING THE STREAMING INDEX


## LIMITATIONS

This feature is still in the early prototype phase and as a result has minimal functionality.

- Only the types INT, DECIMAL and VARCHAR are supported.





