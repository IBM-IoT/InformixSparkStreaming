#include <mi.h>
#include <miami.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

typedef mi_unsigned_char1 mi_uchar;
typedef mi_unsigned_integer mi_uint;
typedef mi_double_precision mi_double;

int socket_file_descriptor;
int newsocket_file_descriptor;
char* informix_directory;
FILE *log_file;
//char buffer[256];

// TODO This is a mess. Whoever inherits this, sorry for the technical debt. I was instructed to create the minimal
// viable prototype and that is what I did. Spent no time on making the code sane.

MI_DECL mi_integer
am_open (mi_pointer *tableDesc)
{
    // According to Lance, the actual socket creation should be moved here instead of in am_create
    openlog("AM_OPEN", 0, LOG_USER);
   // Allow up to 512 rows to be inserted/updated with a single SQL statement.
   // Not sure if this is even needed.
   mi_integer rows = mi_tab_setniorows(tableDesc, 512);
   syslog(LOG_INFO, "mi_tab_setniorows return value %d", rows);
   return 0;
}

MI_DECL mi_integer
am_close (mi_pointer *buf)
{
    return 0;
}

MI_DECL mi_integer
am_create (mi_pointer *buf)
{
    openlog("AM_CREATE", 0, LOG_USER);
    // Open the socket when we create the index.
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor < 0){
        syslog(LOG_ERR, "Error opening socket");
    }
    else{
        syslog(LOG_ERR, "Successfully created socket with file descriptor: %d\n", socket_file_descriptor);
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
    serv_addr.sin_port = htons(13341); // TODO Make this port number set from a config file.


    syslog(LOG_INFO, "addr: %l", serv_addr.sin_addr.s_addr);
    syslog(LOG_INFO, "port: %d\n", serv_addr.sin_port);
    syslog(LOG_INFO, "socket_file_descriptor %d \n", socket_file_descriptor);
    if (bind(socket_file_descriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        syslog(LOG_INFO, "Bind failed");
        syslog(LOG_INFO, "Error number: %d \n", errno);
    }

    listen(socket_file_descriptor,5);
    clilen = sizeof(cli_addr);
    newsocket_file_descriptor = accept(socket_file_descriptor,
                                       (struct sockaddr *) &cli_addr,
                                       &clilen);

    rtn = setsockopt(newsocket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen);
    setsockopt(newsocket_file_descriptor, SOL_SOCKET, SO_REUSEPORT, (void*) &optVal, optLen);

    if (newsocket_file_descriptor < 0) {
        syslog(LOG_INFO, "Accept failed");
        syslog(LOG_INFO, "Error number: %d \n", errno);
    }
    return 0;
}

MI_DECL mi_integer
am_drop (mi_pointer *buf)
{
    close(newsocket_file_descriptor);
    close(socket_file_descriptor);
    return 0;
}

MI_DECL mi_integer
am_insert (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
    openlog("AM_INSERT", 0, LOG_USER);

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
    syslog(LOG_INFO, "Number of rows: %d\n", nrows);

    for (z = 0; z < nrows; z++){  //Pointless, since I can only insert one row at a time anyway.
        mi_tab_nextrow(buf0, &row, &rowid, &fragid);

        numcols = mi_column_count(row);
        for (i = 0; i < numcols; i++){
            mi_string *col_type_name = mi_type_typename(mi_column_typedesc(row, i));
            syslog(LOG_INFO, "Column type: %s\n", col_type_name);


            // Column value is an integer.
            if (strstr("integer", col_type_name)){
                y = mi_value(row, i, &colval, &collen);
                full_int = (mi_integer) colval;
                length = sprintf(buffer, "%d", full_int);
                strcat(buffer, ",");
                syslog(LOG_INFO, "INT value is: %s\n", buffer);
                colval = NULL;
            }
            // Column value is an varchar.
            if (strstr(col_type_name, "varchar")){
                mi_lvarchar *lv_ptr;
                y = mi_value(row, i, &lv_ptr, &collen);
                syslog(LOG_INFO, "LENGTH: %d", collen);
                length = length + collen;
                mi_string *test42 = mi_lvarchar_to_string(lv_ptr);
                strcat(buffer, test42);
                strcat(buffer, ",");
                syslog(LOG_INFO, "STRING VALUE: %s", test42);
            }
            // Column value is an decimal.
            if (strstr(col_type_name, "decimal")){
                mi_decimal *floater;
                y = mi_value(row, i, &floater, &collen);
                length = length + collen;
                mi_string * test = mi_decimal_to_string(floater);
                strcat(buffer, test);
                strcat(buffer, ",");
                syslog(LOG_INFO, "DECIMAL VALUE: %s", test);
            }

        }
         buffer[strlen(buffer)-1] = 0;
         strcat(buffer, "\n");
         syslog(LOG_INFO, "buffer value: %s", buffer);

         // Send column value through socket.
         int index;
         char *e;
         e = strchr(buffer, '\0');
         index = (int)(e - buffer);
         syslog(LOG_INFO, "Buffer length: %d", index);
         int n = write(newsocket_file_descriptor, buffer, index);
         if (n < 0){
            syslog(LOG_INFO, "Write failed");
            syslog(LOG_INFO, "Error number: %d", errno);
         }
    }

return 0;
}

MI_DECL mi_integer
am_getnext (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
  return 0;
}

am_delete (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
    return 0;
}

am_update (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2, mi_pointer *buf3, mi_pointer *buf4)
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

am_endscan (mi_pointer *buf)
{
    return 0;
}

am_beginscan (mi_pointer *buf)
{
    return 0;
}
