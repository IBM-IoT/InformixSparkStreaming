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

/*
 * Tracing-related macros
 */
#define TRACE_CLASS "Pointer"
#define TRACE_LEVEL 20

#define POINTER_TRACE_ENTER(fn) DPRINTF(TRACE_CLASS, TRACE_LEVEL, ("Enter " #fn))
#define POINTER_TRACE_EXIT(fn)  DPRINTF(TRACE_CLASS, TRACE_LEVEL, ("Exit " #fn))
#define POINTER_TRACE(args)     DPRINTF(TRACE_CLASS, TRACE_LEVEL, args)

typedef mi_unsigned_char1   mi_uchar;
typedef mi_unsigned_integer mi_uint;
typedef mi_double_precision mi_double;

/*
MI_DECL mi_pointer *
PointerRecieve (mi_sendrecv *data)
{
  mi_uchar    *datap;
  mi_pointer  *pointer;

  POINTER_TRACE_ENTER("PointerRecieve");

  bbox = mi_alloc (sizeof(mi_pointer));

  datap = (mi_uchar*) mi_get_vardata ((mi_lvarchar*) data);
  datap = mi_get_double_precision (datap, &(bbox->ll.x));
  datap = mi_get_double_precision (datap, &(bbox->ll.y));
  datap = mi_get_double_precision (datap, &(bbox->ll.z));
  datap = mi_get_double_precision (datap, &(bbox->ur.x));
  datap = mi_get_double_precision (datap, &(bbox->ur.y));
  datap = mi_get_double_precision (datap, &(bbox->ur.z));

  POINTER_TRACE_EXIT("PointerRecieve");

  return bbox;
}
*/
MI_DECL mi_sendrecv *
PointerSend (mi_pointer *pointer)
{
  mi_sendrecv *retval;
  mi_uchar    *datap;

  POINTER_TRACE_ENTER("PointerSend");

  retval = (mi_sendrecv*) mi_new_var (sizeof(mi_pointer));
  datap = (mi_uchar*) mi_get_vardata ((mi_lvarchar*) retval);

  datap = mi_put_bytes (datap, *pointer, sizeof(mi_pointer));

  POINTER_TRACE_EXIT("PointerSend");

  return retval;
}

MI_DECL mi_integer
tachyonCreate (mi_pointer *buf)
{

  FILE *f = fopen("/opt/informix/create.txt", "w");
  if (f == NULL)
  {
    printf("Error opening file!\n");
    return 1;
  }

  fprintf(f, "%lx \n", buf);
  fprintf(f, "%lx \n", *buf);

  //long *test = *buf;
  //MI_AM_TABLE_DESC * table = *buf;
 // fprintf(f, "%lx \n", *test);
  /* print some text */
  //mi_pointer *test = *buf;
  mi_string* tableName = mi_tab_name(buf);
  const char *text = "Write this to the file";
  fprintf(f, "Table name: %s\n", tableName);
  //fprintf(f, "test");
  //fprintf(f, "%lx", *buf);

  //MI_AM_TABLE_DESC test = **buf;

  //fprintf(f, "3: %lx", *buf2);


  fclose(f);

 /* long addrAsLong;
  char textbuf[100];
  MI_ROW *row;
  mi_integer result;
  MI_DATUM *value;
  int length;


  FILE *f = fopen("/opt/informix/file.txt", "w");
  if (f == NULL)
  {
  printf("Error opening file!\n");
  return 1;
  }


  POINTER_TRACE_ENTER("tachyonCreate");
  *//* establish server connection *//*
  MI_CONNECTION *conn = mi_open(NULL,NULL,NULL);
  sprintf (textbuf, "execute function createTableInTachyon(%lx);", *buf);
  result = mi_exec(conn, textbuf, MI_QUERY_BINARY);
  result = mi_get_result(conn);
  if (result != MI_NO_MORE_RESULTS) {
  row = mi_next_row(conn, &result);
  result = mi_value(row, 0, value, &length);
  }
  mi_query_finish(conn);
  mi_close(conn);
  POINTER_TRACE_EXIT("tachyonCreate");
  // return (mi_integer) *value;
  fprintf(f, "%lx", *value);
  return (long) *value;*/
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
    FILE *f = fopen("/opt/informix/open.txt", "w");
    if (f == NULL)
    {
      printf("Error opening file!\n");
      return 1;
    }

    mi_integer rows = mi_tab_setniorows(buf, 1);
    fprintf(f, "mi_tab_setniorows return value %d\n", rows);

    fclose(f);

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

bzero((char *) &serv_addr, sizeof(serv_addr));
portno = atoi("9999");
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY;
serv_addr.sin_port = htons(portno);

sockfd = socket(AF_INET, SOCK_STREAM, 0);

MI_ROW *row = NULL;
mi_integer rowid = 0;
mi_integer fragid = 0;

//fprintf(f, " %s\t", colname);
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
    mi_integer y = mi_value(row, i, &colval, &collen);
    full_int = (mi_integer) colval;
    fprintf(f, "%d\t", full_int);


    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
/* if (newsockfd < 0)
 error("ERROR on accept");
 */
 bzero(buffer,256);

 n = write(newsockfd,"I got your message",18);
 //if (n < 0) error("ERROR writing to socket");
 close(newsockfd);
 close(sockfd);
    }

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


mi_integer collen = 0;
MI_DATUM *colval = NULL;
int full_int = 0;
for (i=0; i < numcols; i++){
    mi_integer y = mi_value(buf3, i, &colval, &collen);
    full_int = (mi_integer) colval;
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

MI_DECL mi_pointer *
PointerInput (mi_lvarchar *input_varchar)
{
  mi_pointer *buf;
  mi_char    *input_text;
  mi_integer  n;

  POINTER_TRACE_ENTER("PointerInput");

  /* Allocate space for the pointer being read in */
  buf = mi_alloc (2*sizeof(mi_pointer));
  (*buf) = buf + sizeof(mi_pointer);
  DPRINTF (TRACE_CLASS, TRACE_LEVEL, ("sizeof(mi_pointer) = %d", sizeof(mi_pointer)));

  /* Convert the input */
  input_text = mi_lvarchar_to_string (input_varchar);

  /* Use sscanf to read in the input */
  n = sscanf (input_text, "0x%lx", *buf);

/* Check for errors in the input */
if (n != 1) {
  mi_db_error_raise (NULL, MI_EXCEPTION, "Cannot parse input text", 0);
}

  POINTER_TRACE_EXIT("PointerInput");

  return *buf;
}

MI_DECL mi_lvarchar *
PointerOutput (mi_pointer *buf)
{
  mi_char textbuf[10];

  POINTER_TRACE_ENTER("PointerOutput");

  sprintf (textbuf, "%#lx", *buf);

  POINTER_TRACE_EXIT("PointerOutput");

  return mi_string_to_lvarchar (textbuf);
}
