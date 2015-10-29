# Guide for Future Maintainer/Developer

The Informix Socket Streaming feature is the brainchild of Lance Feagan and he should be your point of contact for 
any high-level issues or questions.

Read through this guide and you should get a clear understanding of the current state of the project and what work 
needs to be done.


Items to complete (in no particular order):
- Add full logging functionality - Simply have the C UDRs write to a log file where-ever appropriate. This should 
include any information relevant to debugging the program and any important information regrading the program's state.
There is already some primitive logging functionality but it needs to be improved upon.
- Setup a configuration file to be read to set things like the port number to be used, output format, etc... 
Anything the user might want to customize should be in here.
- Write up complete documentation.
- Have some C experts look over the code. (Lance Faegan would be a good start). I am not very experienced in C and the
 code should be reviewed by a C expert for any issues before going to production.
 
-Add support for more data types. Currently, only INT, DECIMAL and VARCHAR are supported.
-Add suport for streaming on UPDATE. Currently, only streaming on INSERT is supported. The code needed should be very
 similar.
...