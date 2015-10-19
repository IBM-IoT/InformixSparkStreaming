IN PROGRESS

Congratulations, you just got assigned to what is actually a pretty neat project! I've figured out a lot of the 
plumbing and tough bits already and have a clear vision of what needs to be done to get this project production ready. 

The Informix Socket Streaming feature is the brainchild of Lance Fagen and he should be your point of contact for 
any high-level issues or questions.

Read through this guide and you should get a clear understanding of the current state of the project and what work 
needs to be done.


Items to complete:
1. Add full logging functionality - Simply have the C UDRs write to a log file where-ever appropriate. This should 
include any information relevant to debugging the program and any important information regrading the program's state.
2. Setup a configuration file to be read to set things like the port number to be used, output format, etc... 
Anything the user might want to customize should be in here.
3. Write up complete documentation.
- Have some C experts look over the code. (Lance Fagen would be a good start). I am not very experienced in C and the
 code should be reviewed by a C expert for any issues before going to production.
...