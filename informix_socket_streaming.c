#include <mi.h>
#include <miami.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

typedef mi_unsigned_char1   mi_uchar;
typedef mi_unsigned_integer mi_uint;
typedef mi_double_precision mi_double;

int socket_file_descriptor;
int newsocket_file_descriptor;
char* informix_directory;
FILE *log_file;
//char buffer[256];

MI_DECL mi_integer
tachyonOpen (mi_pointer *tableDesc)
{
    // Create the log file we will use throughout the program.
    log_file = fopen("/opt/informix/tmp/open.log", "w");
    if (log_file == NULL)
    {
        printf("Error creating log file.\n");
    }

   // Allow up to 512 rows to be inserted/updated with a single SQL statement.
   mi_integer rows = mi_tab_setniorows(tableDesc, 512);
   fprintf(log_file, "mi_tab_setniorows return value %d\n", rows);
   fclose(log_file);
   return 0;
}

MI_DECL mi_integer
tachyonClose (mi_pointer *buf)
{
    return 0;
}

MI_DECL mi_integer
tachyonCreate (mi_pointer *buf)
{
    log_file = fopen("/opt/informix/tmp/create.log", "w");
    // Open the socket when we create the index.
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor < 0){
        fprintf(log_file, "Error opening socket.\n");
    }
    else{
        fprintf(log_file, "Successfully created socket with file descriptor: %d\n", socket_file_descriptor);
    }

    const int       optVal = 1;
    const socklen_t optLen = sizeof(optVal);

    int rtn = setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen);
    setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEPORT, (void*) &optVal, optLen);

    int portno;
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;

    bzero((char *) &serv_addr, sizeof(serv_addr));


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(13341);
    fprintf(log_file, "addr: %l\n", serv_addr.sin_addr.s_addr);
    fprintf(log_file, "port: %d\n", serv_addr.sin_port);
    fprintf(log_file, "socket_file_descriptor %d \n", socket_file_descriptor);
    if (bind(socket_file_descriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(log_file, "Bind failed \n");
        fprintf(log_file, "errno: %d \n", errno);
    }

    listen(socket_file_descriptor,5);
    clilen = sizeof(cli_addr);
    newsocket_file_descriptor = accept(socket_file_descriptor,
                                       (struct sockaddr *) &cli_addr,
                                       &clilen);

    rtn = setsockopt(newsocket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen);
    setsockopt(newsocket_file_descriptor, SOL_SOCKET, SO_REUSEPORT, (void*) &optVal, optLen);

    if (newsocket_file_descriptor < 0) {
        fprintf(log_file, "accept failed\n");
        fprintf(log_file, "errno: %d \n", errno);
    }
    fclose(log_file);
    return 0;
}

MI_DECL mi_integer
tachyonDrop (mi_pointer *buf)
{
    close(newsocket_file_descriptor);
    close(socket_file_descriptor);
    return 0;
}

MI_DECL mi_integer
tachyonInsert (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
    int n;
    log_file = fopen("/opt/informix/tmp/insert.log", "w");
    if (log_file == NULL)
    {
        printf("Error opening file!\n");
    }


    //bzero(buffer, 256);

    MI_ROW *row = NULL;
    char buffer[100];
    mi_integer rowid = 0;
    mi_integer fragid = 0;
    mi_integer x = 0;
    mi_integer numcols = 0;
    mi_string *col_type_name;
    mi_integer collen = 0;
    MI_DATUM colval = NULL;
    int full_int = 0;
    int z = 0;
    int i = 0;
    int length = 0;
     mi_integer y = 0;

    // Get the 
    mi_integer nrows = mi_tab_niorows(buf0);
    fprintf(log_file, "Number of rows: %d\n", nrows);

    for (z = 0; z < nrows; z++){  //Pointless, since I can only insert one row at a time anyway.
        mi_tab_nextrow(buf0, &row, &rowid, &fragid);

        numcols = mi_column_count(row);
        for (i = 0; i < numcols; i++){
            mi_string *col_type_name = mi_type_typename(mi_column_typedesc(row, i));
            //fprintf(log_file, "%s\t", col_type_name);
            fprintf(log_file, "COLUMN TYPE:%s\n", col_type_name);
             //y = mi_value(row, i, &colval, &collen);


            // Column value is an integer.
            if (strstr("integer", col_type_name)){
                y = mi_value(row, i, &colval, &collen);
                fprintf(log_file, "PROCESS INTEGER:\n");
                full_int = (mi_integer) colval;
                length = sprintf(buffer, "%d", full_int);
                strcat(buffer, ",");
                fprintf(log_file, "INT VALUE: %s\n", buffer);
                colval = NULL;

            }
          // Column value is an varchar.
            if (strstr(col_type_name, "varchar")){
                mi_lvarchar *lv_ptr;
                fprintf(log_file, "PROCESSING STRING: \n");
                y = mi_value(row, i, &lv_ptr, &collen);

                fprintf(log_file, "LENGTH: %d\n", collen);
                length = length + collen;
                mi_string *test42 = mi_lvarchar_to_string(lv_ptr);
                strcat(buffer, test42);
                strcat(buffer, ",");
                fprintf(log_file, "STRING VALUE: %s\n",test42);
            }
            // Column value is an decimal.
            if (strstr(col_type_name, "decimal")){
                mi_decimal *floater;
                fprintf(log_file, "PROCESS DECIMAL:\n");
                y = mi_value(row, i, &floater, &collen);
                length = length + collen;
                mi_string * test = mi_decimal_to_string(floater);
                strcat(buffer, test);
                strcat(buffer, ",");
                fprintf(log_file, "DECIMAL VALUE: %s\n", test) ;
                //fprintf(log_file, "LENGTH: %d\n", collen);
            }



        }
         buffer[strlen(buffer)-1] = 0;
         strcat(buffer, "\n");
         fprintf(log_file, "BUFFER: %s", buffer);
         // Send column value through socket.


         int index;
         char *e;
         e = strchr(buffer, '\0');
         index = (int)(e - buffer);
         fprintf(log_file, "LENGTH: %d", index);
         n = write(newsocket_file_descriptor, buffer, index);
         if (n < 0){
             fprintf(log_file, "Write failed \n");
             fprintf(log_file, "errno: %d \n", errno);
         }
    }
    //close(newsocket_file_descriptor);
    //close(socket_file_descriptor);

    fclose(log_file);

return 0;
}

MI_DECL mi_integer
tachyonGetByid (mi_pointer *buf)
{
  return 0;
}

MI_DECL mi_integer
tachyonGetNext (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
  return 0;
}

tachyonDelete (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
    return 0;
}

tachyonUpdate (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2, mi_pointer *buf3, mi_pointer *buf4)
{
/*    FILE *f = fopen("/opt/informix/update.txt", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        return -1;
    }


    int socket_file_descriptor, newsocket_file_descriptor, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor < 0) fprintf(f, "Error opening socket \n");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(12517);
    fprintf(f, "addr: %l\n", serv_addr.sin_addr.s_addr);
    fprintf(f, "port: %d\n", serv_addr.sin_port);
    fprintf(f, "socket_file_descriptor %d \n", socket_file_descriptor);
    if (bind(socket_file_descriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(f, "Bind failed \n");
        fprintf(f, "errno: %d \n", errno);
    }


    listen(socket_file_descriptor,5);
    clilen = sizeof(cli_addr);
    newsocket_file_descriptor = accept(socket_file_descriptor,
                       (struct sockaddr *) &cli_addr,
                       &clilen);


    if (newsocket_file_descriptor < 0) {
    fprintf(f, "accept failed\n");
    fprintf(f, "errno: %d \n", errno);
    }

    bzero(buffer,256);

    mi_integer numcols;
    numcols = mi_column_count(buf3);
    fprintf(f, "Number of columns: %d\n", numcols);

    mi_string *colname;
    int i = 0;
    while( i < numcols )
    {
        colname = mi_column_name(buf3, i);
        fprintf(f, " %s\t", colname);
        i++;
    }

    for (i=0; i < numcols; i++){
        char buffer[100];

        mi_integer y = mi_value(row, i, &colval, &collen);
        full_int = (mi_integer) colval;
        int length = sprintf(buffer, "%d", full_int);
        strcat(buffer, "\n");
        n = write(newsocket_file_descriptor, buffer, length + 1);

        if (n < 0){
            fprintf(f, "Write failed \n");
            fprintf(f, "errno: %d \n", errno);
        }
        fprintf(f, "%s\t", buffer);
    }


    mi_integer collen = 0;
    MI_DATUM *colval = NULL;
    int full_int = 0;
    for (i=0; i < numcols; i++){
        char buffer[100];
        mi_integer y = mi_value(buf3, i, &colval, &collen);
        full_int = (mi_integer) colval;
        int length = sprintf(buffer, "%d", full_int);
        strcat(buffer, "\n");
        n = write(newsocket_file_descriptor, buffer, length + 1);
        fprintf(f, "%d\t", full_int);
    }
    fclose(f);*/
    return 0;
}

tachyonEndscan (mi_pointer *buf)
{
    return 0;
}

tachyonBeginscan (mi_pointer *buf)
{
    return 0;
}
