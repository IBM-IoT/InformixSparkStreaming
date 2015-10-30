# TODO

The Informix Socket Streaming feature is the brainchild of Lance Feagan and he should be your point of contact for 
any high-level issues or questions.


## Items to complete (in no particular order):
- Detup a configuration file to be read to set things like the port number to be used, output format, etc... 
- Write up complete documentation.
- Have some C experts look over the code. (Lance Feagan would be a good start). I am not very experienced in C and the
 code should be reviewed by a C expert for any issues before going to production.
- Add support for more data types. Currently, only INT, DECIMAL and VARCHAR are supported.
- Add suport for streaming on UPDATE. Currently, only streaming on INSERT is supported. The code needed should be very
 similar to that used in INSERT
- Figure out how to handle multiple indexes
- Figure out how to streaming on multiple columns within a single index (if we even want this)
- Better handle the opening and closing of sockets. How best to handle sockets with database being opened and re-opened.
- Overall socket management, how to handle sockets that are already in use, etc...