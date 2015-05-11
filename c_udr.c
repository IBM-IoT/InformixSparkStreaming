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


MI_DECL mi_integer
tachyonCreate (mi_pointer *buf)
{
  return 0;
}

MI_DECL mi_integer
tachyonDrop (mi_pointer *buf)
{
  return 0;
}

MI_DECL mi_integer
tachyonOpen (mi_pointer *buf)
{
  return 0;
}

MI_DECL mi_integer
tachyonClose (mi_pointer *buf)
{
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

MI_DECL mi_integer
tachyonInsert (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
FILE *f = fopen("/opt/informix/insert.txt", "w");
  if (f == NULL)
  {
    printf("Error opening file!\n");
    return -1;
  }

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) fprintf(f, "Error opening socket \n");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(12517);
    fprintf(f, "addr: %l\n", serv_addr.sin_addr.s_addr);
    fprintf(f, "port: %d\n", serv_addr.sin_port);
    fprintf(f, "sockfd %d \n", sockfd);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(f, "Bind failed \n");
        fprintf(f, "errno: %d \n", errno);
    }


    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);


    if (newsockfd < 0) {
        fprintf(f, "accept failed\n");
        fprintf(f, "errno: %d \n", errno);
    }

    bzero(buffer,256);

    MI_ROW *row = NULL;
    mi_integer rowid = 0;
    mi_integer fragid = 0;

    mi_integer nrows = mi_tab_niorows(buf0);
    fprintf(f, "Number of rows: %d\n", nrows);

    mi_integer x = mi_tab_nextrow(buf0, &row, &rowid, &fragid);

    mi_integer numcols;
    numcols = mi_column_count(row);
    fprintf(f, "Number of columns: %d\n", numcols);

    mi_string *colname;
    int i = 0;
    while( i < numcols )
    {
        colname = mi_column_name(row, i);
        fprintf(f, " %s\t", colname);
        i++;
    }

    mi_integer collen = 0;
    MI_DATUM *colval = NULL;
    int full_int = 0;
    for (i=0; i < numcols; i++){
        char buffer[100];

        mi_integer y = mi_value(row, i, &colval, &collen);
        full_int = (mi_integer) colval;
        int length = sprintf(buffer, "%d", full_int);
        strcat(buffer, "\n");
        n = write(newsockfd, buffer, length + 1);

        if (n < 0){
            fprintf(f, "Write failed \n");
            fprintf(f, "errno: %d \n", errno);
        }
        fprintf(f, "%s\t", buffer);
    }

    close(newsockfd);
    close(sockfd);

    fclose(f);

    return 0;
}


tachyonDelete (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2)
{
    return 0;
}

tachyonUpdate (mi_pointer *buf0, mi_pointer *buf1, mi_pointer *buf2, mi_pointer *buf3, mi_pointer *buf4)
{
    FILE *f = fopen("/opt/informix/update.txt", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        return -1;
    }


    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) fprintf(f, "Error opening socket \n");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(12517);
    fprintf(f, "addr: %l\n", serv_addr.sin_addr.s_addr);
    fprintf(f, "port: %d\n", serv_addr.sin_port);
    fprintf(f, "sockfd %d \n", sockfd);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        fprintf(f, "Bind failed \n");
        fprintf(f, "errno: %d \n", errno);
    }


    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);


    if (newsockfd < 0) {
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
        n = write(newsockfd, buffer, length + 1);

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
        n = write(newsockfd, buffer, length + 1);
        fprintf(f, "%d\t", full_int);
    }
    fclose(f);
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
