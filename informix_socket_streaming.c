#include <mi.h>
#include <miami.h>
#include <minmprot.h>
#include <sqltypes.h>

#include <limits.h>
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
#include <float.h>

#include "mqttnet.h"

#ifdef WITH_ISS_DEBUG
  #define ISSDEBUG(x) x
#else
  #define ISSDEBUG(x)
#endif

#define AMPARAM_TOKEN_DELIMITERS " =,"
#define INDEX_LIST_MEMNAME "ISSIndexList"
#define PAYLOAD_LIST_MEMNAME "ISSPayloadList"
#define MQTT_MAX_PACKET_ID 65535

mi_real *constantScanCost = NULL;

typedef struct _ISS_LinkedList {
  void *payload;
  struct _ISS_LinkedList *next;
} ISS_LinkedList;

typedef struct {
  char *host;
  int port;
  int refCount;
  char *topic;
} ISS_ServerInfo;

typedef struct {
  MqttClient *client;
  ISS_ServerInfo *serverInfo;
  int indexCount;
  int pid;
} ISS_MQTTSettings;

typedef struct {
  char *mqttTopic, *indexName;
  ISS_ServerInfo *serverInfo;
  ISS_LinkedList *mqttList;
} ISS_Index;

typedef struct _endxact_payload {
  ISS_MQTTSettings *mqtt;
  char *topic;
  char *payload;
  struct _endxact_payload *next;
} ENDXACT_PAYLOAD;

ISS_LinkedList **indexList = NULL;//, *mqttList = NULL;
ENDXACT_PAYLOAD **endxact_payload = NULL;
int nextMQTTClientID = 0;

int mqttLastPacketID = 0;


ENDXACT_PAYLOAD* xact_payload_new( char *payload , ISS_MQTTSettings *mqtt, char *topic)
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Function %s: payload is %s\n" , __FUNCTION__ , payload);)
  ENDXACT_PAYLOAD *newxact = (ENDXACT_PAYLOAD*)mi_dalloc( sizeof(ENDXACT_PAYLOAD) , PER_SYSTEM );
  newxact->mqtt = mqtt;
  newxact->topic = topic;
  newxact->payload = payload;
  newxact->next = NULL;
  ISSDEBUG(syslog( LOG_INFO, "Function %s: newxact->payload is %s\n" , __FUNCTION__ , newxact->payload);)
  return newxact;
}


ENDXACT_PAYLOAD* xact_payload_add( ENDXACT_PAYLOAD *list, ENDXACT_PAYLOAD *link )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Begin %s:\n" , __FUNCTION__ );)
   if( list == NULL )
   {
      list = link;
      link->next = NULL;
   }
   else
   {
      link->next = list;
   }
   ISSDEBUG(syslog( LOG_INFO, "Function %s: list->payload is %s\n" , __FUNCTION__ , list->payload);)
   ISSDEBUG(syslog( LOG_INFO, "Function %s: link->payload is %s\n" , __FUNCTION__ , link->payload);)

   return link;
}

word16 mqttGetNextPacketID()
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  mqttLastPacketID = ( mqttLastPacketID >= MQTT_MAX_PACKET_ID ) ? 1 : mqttLastPacketID + 1;
  return mqttLastPacketID;
}

ISS_LinkedList* ISS_LinkedList_remove( ISS_LinkedList *list, ISS_LinkedList *link )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  if( list == link )
  {
    return list->next;
  }

  ISS_LinkedList *prev = list, *current = list->next;
  while( current != NULL )
  {
    if( link == current )
    {
      prev->next = current->next;
      break;
    }

    prev = current;
    current = current->next;
  }

  return list;
}

ISS_LinkedList* ISS_LinkedList_add( ISS_LinkedList *list, ISS_LinkedList *link )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  if( list == NULL )
  {
    list = link;
    link->next = NULL;
  }
  else
  {
    link->next = list;
  }

  return link;
}

ISS_LinkedList* ISS_LinkedList_new( void *payload )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  ISS_LinkedList *newLink = (ISS_LinkedList*)mi_dalloc( sizeof(ISS_LinkedList) , PER_SYSTEM );
  newLink->payload = payload;
  newLink->next = NULL;
  return newLink;
}

mi_integer getTableName( mi_string *indexName, char *dest )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  MI_CONNECTION *conn = mi_open( NULL, NULL, NULL );
  if( !conn ) return MI_ERROR;
  mi_string queryString[256];
  sprintf( queryString , "select tabname from sysindices join systables on sysindices.tabid = systables.tabid where sysindices.idxname = '%s'" , indexName );
  if( mi_exec( conn , queryString , MI_QUERY_BINARY ) == MI_ERROR ) return MI_ERROR;
  if( mi_get_result( conn ) != MI_ROWS ) return MI_ERROR;

  mi_integer error = 0;
  MI_ROW *row = mi_next_row( conn , &error );
  if( !row ) return MI_ERROR;

  MI_DATUM valueBuffer = 0;
  mi_integer valueLen = 0;
  if( mi_value( row , 0 , &valueBuffer , &valueLen ) != MI_NORMAL_VALUE ) return MI_ERROR;

  mi_string *tableName = mi_lvarchar_to_string( (mi_lvarchar*)valueBuffer );
  strcat( dest , tableName );
  mi_free( tableName );

  return MI_OK;
}

ISS_ServerInfo* getMQTTServerInfo( MI_AM_TABLE_DESC *tableDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  mi_string *params = mi_tab_amparam( tableDesc );
  ISSDEBUG(syslog( LOG_INFO, "Function %s: AM Params: %s\n" , __FUNCTION__ , params );)

  ISS_ServerInfo *info = NULL;

  if( params )
  {
    char *serverHost = NULL;
    char *serverTopic = NULL;
    int serverPort = 0;
    char *paramName = strtok( params, AMPARAM_TOKEN_DELIMITERS );
    char *paramValue = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
    while( paramName )
    {
      ISSDEBUG(syslog( LOG_INFO, "Function %s: Param is | %s | Value is: | %s |\n" , __FUNCTION__ , paramName , paramValue );)
      if( !paramValue )
      {
        ISSDEBUG(syslog( LOG_INFO, "Function %s: Missing param value for: %s\n" , __FUNCTION__ , paramName );)
        break;
      }

      if( strcmp( paramName , "host" ) == 0 )
      {
        int len = strlen( paramValue );
        serverHost = (char*)mi_dalloc( len , PER_SYSTEM );
        memcpy( serverHost , paramValue , len );
        serverHost[ len ] = 0;
      }
      else if( strcmp( paramName , "port" ) == 0 )
      {
        int len = strlen( paramValue );
        if( len > 5 ) break;

        serverPort = atoi( paramValue );
        if( serverPort < 1 ) break;
      }
      else if( strcmp( paramName , "topic" ) == 0 )
      {
        int len = strlen( paramValue );
        serverTopic = (char*)mi_dalloc( len , PER_SYSTEM );
        memcpy( serverTopic , paramValue , len );
        serverTopic[ len ] = 0;
      }
      else
      {
        ISSDEBUG(syslog( LOG_INFO, "Function %s: Unknown parameter name: %s\n" , __FUNCTION__ , paramName );)
        /* continue; */
      }

      paramName = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
      paramValue = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
    }

    if( serverHost && serverPort > 0 )
    {
      info = (ISS_ServerInfo*)mi_dalloc( sizeof( ISS_ServerInfo ) , PER_SYSTEM );
      info->host = serverHost;
      info->port = serverPort;
      info->refCount = 0;
      info->topic = serverTopic;

      ISSDEBUG(syslog( LOG_INFO, "Function %s: MQTT Server Info: %s:%d Topic=%s\n" , __FUNCTION__ , info->host, info->port, info->topic );)
    }
    else if( serverHost ) mi_free( serverHost );
  }
  else
  {
    ISSDEBUG(syslog( LOG_INFO, "Function %s: No parameters supplied.\n" , __FUNCTION__ );)
    return NULL;
  }

  return info;
}

ISS_MQTTSettings* getMQTTSettings( ISS_ServerInfo *serverInfo )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  ISS_MQTTSettings *settings = NULL;
  ISS_LinkedList *currentIndex = *indexList, *currentMQTT = NULL;
  int pid = getpid();
  while( currentIndex != NULL )
  {
    currentMQTT = ((ISS_Index*)currentIndex->payload)->mqttList;
    while( currentMQTT != NULL )
    {
      settings = (ISS_MQTTSettings*)currentMQTT->payload;
      if( settings->pid == pid && settings->serverInfo == serverInfo ) break;
      settings = NULL;
      currentMQTT = currentMQTT->next;
    }
    if( settings ) break;
    currentIndex = currentIndex->next;
  }

  if( settings == NULL )
  {
    settings = (ISS_MQTTSettings*)mi_dalloc( sizeof(ISS_MQTTSettings) , PER_SYSTEM );
    settings->serverInfo = serverInfo;
    settings->client = NULL;
    settings->indexCount = 0;
    settings->pid = pid;
  }

  return settings;
}

void removeMQTTSettings( ISS_MQTTSettings *settings )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

    if( settings != NULL )
    {
      ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Removing MQTT Settings: %d,%s:%d\n" , __FUNCTION__ , getpid(), settings->pid, settings->serverInfo->host, settings->serverInfo->port );)

      if( settings->client )
      {
        if( settings->client->flags & MQTT_CLIENT_FLAG_IS_CONNECTED )
        {
          MqttClient_Disconnect( settings->client );
          MqttClient_NetDisconnect( settings->client );
        }
        mi_free( settings->client->tx_buf );
        mi_free( settings->client->rx_buf );
        mi_free( settings->client->net );
        mi_free( settings->client );
      }

      ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Client disconnected/destroyed...\n" , __FUNCTION__ , getpid() );)

      mi_free( settings );
    }
}

mi_integer connectMQTTClient( ISS_MQTTSettings *mqttSettings )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  if( !mqttSettings->client || ( mqttSettings->client->flags & MQTT_CLIENT_FLAG_IS_CONNECTED ) == 0 )
  {
    int rc;
    MqttConnect connect;

    if( !mqttSettings->client )
    {
      ISSDEBUG(syslog( LOG_INFO , "Function %s: [%d] MQTT Client does not exist.\n" , __FUNCTION__ , getpid() );)

      MqttNet *net = (MqttNet*)mi_dalloc( sizeof( MqttNet ) , PER_SYSTEM );
      rc = MqttClientNet_Init( net );
      if( rc != 0 )
      {
        ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] MQTT Net Init failed: %s\n" , __FUNCTION__ , getpid(), MqttClient_ReturnCodeToString( rc ) );)
        mi_free( net );
        return MI_ERROR;
      }

      mqttSettings->client = (MqttClient*)mi_dalloc( sizeof( MqttClient ) , PER_SYSTEM );
      byte *txBuffer = (byte*)mi_dalloc( 1024 , PER_SYSTEM );
      byte *rxBuffer = (byte*)mi_dalloc( 1024 , PER_SYSTEM );

      rc = MqttClient_Init( mqttSettings->client , net , NULL , txBuffer , 1024 , rxBuffer , 1024 , 1000 );
      if( rc == 0 ) rc = MqttClient_NetConnect( mqttSettings->client , mqttSettings->serverInfo->host , mqttSettings->serverInfo->port , 5000 , 0 , NULL );

      if( rc != 0 )
      {
        ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] MQTT Connect failed: %s\n" , __FUNCTION__ , getpid(), MqttClient_ReturnCodeToString( rc ) );)

        mi_free( txBuffer );
        mi_free( rxBuffer );
        mi_free( mqttSettings->client );
        mqttSettings->client = NULL;
        return MI_ERROR;
      }
    }
    else
    {
      ISSDEBUG(syslog( LOG_INFO , "Function %s: [%d] MQTT Client not connected.\n" , __FUNCTION__ , getpid() );)
    }

    memset( &connect , 0 , sizeof( MqttConnect ) );
    connect.keep_alive_sec = 0;
    connect.clean_session = 1;

    char mqttClientID[32];
    memset( mqttClientID , 0 , 32 );
    sprintf( mqttClientID , "iss%d_%d" , getpid(), nextMQTTClientID++ );

    connect.client_id = mqttClientID;
    rc = MqttClient_Connect( mqttSettings->client , &connect );
    if( rc != 0 )
    {
      ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] MQTT Connect failed: %s\n" , __FUNCTION__ , getpid(), MqttClient_ReturnCodeToString( rc ) );)
      return MI_ERROR;
    }

    ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] MQTT Client connected\n" , __FUNCTION__ , getpid() );)
  }

  return MI_OK;
}

ISS_Index* getIndex( MI_AM_TABLE_DESC *tableDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  if( indexList == NULL ) return NULL;

  if (tableDesc == NULL)
  {
     ISSDEBUG(syslog( LOG_INFO, "Function %s: tableDesc is NULL.\n" , __FUNCTION__ );)
  }

  mi_string *indexName = mi_tab_name( tableDesc );
  ISS_LinkedList *current = *indexList;
  ISS_Index *index = NULL;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: check 1.\n" , __FUNCTION__ );)
  ISSDEBUG(syslog( LOG_INFO, "Function %s: Index name is %s.\n" , __FUNCTION__ , indexName );)
  while( current != NULL )
  {
    index = (ISS_Index*)current->payload;
    if( strcmp( index->indexName , indexName ) == 0 ) break;
    index = NULL;
    current = current->next;
  }

  ISSDEBUG(syslog( LOG_INFO, "Function %s: check 2.\n" , __FUNCTION__ );)
  if( index == NULL )
  {
    ISS_ServerInfo *serverInfo = getMQTTServerInfo( tableDesc );
    if( serverInfo == NULL ) return NULL;

    current = *indexList;
    while( current != NULL )
    {
      index = (ISS_Index*)current->payload;
      if( strcmp( index->serverInfo->host , serverInfo->host ) == 0 && index->serverInfo->port == serverInfo->port && strcmp( index->serverInfo->topic , serverInfo->topic ) == 0 )
      {
        ISSDEBUG(syslog( LOG_INFO, "Function %s: index->serverInfo->topic is %s\n" , __FUNCTION__ , index->serverInfo->topic );)
        ISSDEBUG(syslog( LOG_INFO, "Function %s: serverInfo->topic is %s\n" , __FUNCTION__ , serverInfo->topic );)
        mi_free( serverInfo->host );
        mi_free( serverInfo );
        serverInfo = index->serverInfo;
        break;
      }
      current = current->next;
    }

    int indexNameLen = strlen( indexName );

    index = (ISS_Index*)mi_dalloc( sizeof(ISS_Index) , PER_SYSTEM );
    index->mqttTopic = (char*)mi_dalloc( sizeof(char) * 256 , PER_SYSTEM );
    index->serverInfo = serverInfo;
    index->serverInfo->refCount++;
    index->indexName = (char*)mi_dalloc( sizeof(char) * ( indexNameLen + 1 ) , PER_SYSTEM );
    memset( index->mqttTopic , 0 , 256 );
    memcpy( index->indexName , indexName , indexNameLen );
    index->indexName[ indexNameLen ] = 0;
    getTableName( indexName , index->mqttTopic );
    index->mqttList = NULL;

    *indexList = ISS_LinkedList_add( *indexList , ISS_LinkedList_new( index ) );

    ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Added index: %s\n" , __FUNCTION__ , getpid(), indexName );)
  }

  return index;
}

ISS_MQTTSettings* getMQTTClient( ISS_Index *index )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  int pid = getpid();
  ISS_MQTTSettings *settings = NULL;
  ISS_LinkedList *current = index->mqttList;
  while( current != NULL )
  {
    settings = (ISS_MQTTSettings*)current->payload;
    if( settings->pid == pid ) break;
    settings = NULL;
    current = current->next;
  }

  if( settings == NULL )
  {
    settings = getMQTTSettings( index->serverInfo );
    if( settings == NULL ) return NULL;

    settings->indexCount++;
    index->mqttList = ISS_LinkedList_add( index->mqttList , ISS_LinkedList_new( settings ) );
  }

  if( connectMQTTClient( settings ) != MI_OK ) return NULL;

  return settings;
}

void removeIndex( mi_string *indexName )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  if( indexList == NULL ) return;

  mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );

  ISS_Index *index = NULL;
  ISS_LinkedList *current = *indexList;
  while( current != NULL )
  {
    index = (ISS_Index*)current->payload;
    if( strcmp( index->indexName , indexName ) == 0 ) break;
    index = NULL;
    current = current->next;
  }

  if( index != NULL )
  {
    *indexList = ISS_LinkedList_remove( *indexList , current );
    mi_free( current );

    ISSDEBUG(syslog( LOG_INFO, "Function %s: Removing index: %s\n" , __FUNCTION__ , index->indexName );)

    ISS_MQTTSettings *settings = NULL;
    ISS_LinkedList *currentMQTT = index->mqttList, *prevMQTT = NULL;
    while( currentMQTT != NULL )
    {
      settings = (ISS_MQTTSettings*)currentMQTT->payload;
      settings->indexCount--;
      if( settings->indexCount <= 0 )
      {
        removeMQTTSettings( settings );
      }

      ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Removing MQTT Link...\n" , __FUNCTION__ , getpid() );)

      prevMQTT = currentMQTT;
      currentMQTT = currentMQTT->next;
      mi_free( prevMQTT );

      ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Done.\n" , __FUNCTION__ , getpid() );)
    }

    index->serverInfo->refCount--;
    if( index->serverInfo->refCount <= 0 )
    {
      ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Removing ServerInfo %s:%d\n" , __FUNCTION__ , getpid(), index->serverInfo->host, index->serverInfo->port );)
      mi_free( index->serverInfo->host );
      mi_free( index->serverInfo );
    }

    mi_free( index->mqttTopic );
    mi_free( index->indexName );
    mi_free( index );

    ISSDEBUG(syslog( LOG_INFO, "Function %s: [%d] Index removed.\n" , __FUNCTION__ , getpid() );)
  }

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
}

void columnValueToString( MI_ROW *row, mi_integer index, char *dest )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  MI_DATUM valueBuffer;
  mi_integer valueLen = 0;
  mi_string *stringValue;

  mi_integer columnTypeID = *(mi_integer*)mi_column_type_id( (MI_ROW_DESC*)row , index );

  mi_integer valueType = mi_value( row ,  index , &valueBuffer , &valueLen );

  if( valueType == MI_NORMAL_VALUE )
  {
    switch( columnTypeID & TYPEIDMASK )
    {
      case SQLSMINT:
      case SQLINT:
      case SQLSERIAL:
        sprintf( dest, "%d", *(mi_integer*)(&valueBuffer) );
        break;
      case SQLSMFLOAT:
        sprintf( dest, "%.*f", FLT_DIG, *(mi_real*)valueBuffer );
        break;
      case SQLFLOAT:
        sprintf( dest, "%.*f", DBL_DIG, *(mi_double_precision*)valueBuffer );
        break;
      case SQLMONEY:
        stringValue = mi_money_to_string( (mi_money*)valueBuffer );
        strcat( dest , stringValue );
        mi_free( stringValue );
        break;
      case SQLDECIMAL:
        stringValue = mi_decimal_to_string( (mi_decimal*)valueBuffer );
        strcat( dest , stringValue );
        mi_free( stringValue );
        break;
      case SQLDATE:
        stringValue = mi_date_to_string( *(mi_date*)(&valueBuffer) );
        strcat( dest , stringValue );
        mi_free( stringValue );
        break;
      case SQLDTIME:
        stringValue = mi_datetime_to_string( (mi_datetime*)valueBuffer );
        strcat( dest , stringValue );
        mi_free( stringValue );
        break;
      case SQLCHAR:
      case SQLVCHAR:
        stringValue = mi_lvarchar_to_string( (mi_lvarchar*)valueBuffer );
        strcat( dest , "\"" );
        strcat( dest , stringValue );
        strcat( dest , "\"" );
        mi_free( stringValue );
        break;
      default:
        ISSDEBUG(syslog( LOG_INFO, "Function %s: Unknown column type ID: %d\n" , __FUNCTION__ , columnTypeID & TYPEIDMASK );)
        break;
    }
  }
  else
  {
    ISSDEBUG(syslog( LOG_INFO, "Function %s: Unknown value type\n" , __FUNCTION__ );)
  }
}

void rowToCSV( MI_ROW *row, char *dest )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  char *offset = dest;

  mi_integer numCols = mi_column_count( (MI_ROW_DESC*)row );
  mi_integer i = 0;
  for( ; i < numCols; i++ )
  {
    columnValueToString( row , i , offset );

    if( i < numCols - 1 ) strcat( offset , "," );
    offset = strchr( offset , 0 );
  }
}

mi_integer am_create( MI_AM_TABLE_DESC *tableDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Function %s: Entering function am_create\n" , __FUNCTION__ );)
  ISSDEBUG(mi_string *indexName = mi_tab_name( tableDesc );)
  ISSDEBUG(syslog( LOG_INFO, "Function %s: Index created: %s\n" , __FUNCTION__ , indexName );)

  return MI_OK;
}

mi_integer am_drop( MI_AM_TABLE_DESC *tableDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Entering function %s\n" , __FUNCTION__ );)

  mi_string *indexName = mi_tab_name( tableDesc );

  removeIndex( indexName );
  ISSDEBUG(syslog( LOG_INFO, "Function %s: Index dropped: %s\n" , __FUNCTION__ , indexName );)

  return MI_OK;
}

mi_integer am_open( MI_AM_TABLE_DESC *tableDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Begin %s:\n" , __FUNCTION__ );)

   if( indexList == NULL )
   {
      int rc = mi_named_get( INDEX_LIST_MEMNAME, PER_SYSTEM, (void**)&indexList );

      if( rc == MI_ERROR )
      {
         ISSDEBUG(syslog( LOG_INFO, "Function %s: Error getting indexList.\n" , __FUNCTION__ );)
      }

      if( rc == MI_NO_SUCH_NAME )
      {
         rc = mi_named_alloc( sizeof( ISS_LinkedList* ), INDEX_LIST_MEMNAME, PER_SYSTEM, (void**)&indexList );
         if( rc == MI_ERROR )
         {
            ISSDEBUG(syslog( LOG_INFO, "Function %s: Error allocating indexList.\n" , __FUNCTION__ );)
         }
         *indexList = NULL;
      }
   }

   if ( endxact_payload == NULL )
   {
      ISSDEBUG(syslog( LOG_INFO, "Function %s: Allocating memory for endxact_payload.\n" , __FUNCTION__ );)
      int rc = mi_named_alloc( sizeof( ENDXACT_PAYLOAD* ), PAYLOAD_LIST_MEMNAME, PER_SYSTEM, (void**)&endxact_payload );
      if( rc == MI_ERROR )
      {
         ISSDEBUG(syslog( LOG_INFO, "Function %s: Error allocating endxact_payload.\n" , __FUNCTION__ );)
      }
      ISSDEBUG(syslog( LOG_INFO, "Function %s: Memory successfully allocated for endxact_payload.\n" , __FUNCTION__ );)
      *endxact_payload = NULL;
   }

   return MI_OK;
}

mi_integer am_close( MI_AM_TABLE_DESC *tableDesc )
{
  return MI_OK;
}

mi_integer am_insert( MI_AM_TABLE_DESC *tableDesc, MI_ROW *row, MI_AM_ROWID_DESC *ridDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Begin %s:\n" , __FUNCTION__ );)

  if( indexList == NULL ) return MI_OK;
  if( mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM ) != MI_OK )
  {
    ISSDEBUG(syslog( LOG_INFO, "Function %s: Error locking memory...\n" , __FUNCTION__ );)
    return MI_OK;
  }

  ISS_Index *index;
  ISS_MQTTSettings *mqtt;
  char *payload;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: Inserting row into table...\n" , __FUNCTION__ );)

  if( ( index = getIndex( tableDesc ) ) == NULL ||
      ( mqtt = getMQTTClient( index ) ) == NULL ||
      ( payload = (char*)mi_dalloc( sizeof( char ) * 1024 , PER_TRANSACTION ) ) == NULL )
  {
    mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
    return MI_OK;
  }

  /* callback function prototype */
  MI_CALLBACK_HANDLE      *cback=NULL;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: Registering callback routine.\n" , __FUNCTION__ );)

  MI_CALLBACK_STATUS MI_PROC_CALLBACK am_eot_cb(MI_EVENT_TYPE type,MI_CONNECTION *conn,void *server_data,void *user_data);
  /* Register the MI_EVENT_COMMIT_ABORT callback handler. */
  cback = mi_register_callback (
        NULL,                  /* register on NULL conn handle  */
        MI_EVENT_COMMIT_ABORT, /* event = end of transaction    */
        am_eot_cb,             /* function server will dispatch */
        NULL,      /* user buffer */
        NULL);                 /* not used */

  if(cback == (MI_CALLBACK_HANDLE *) NULL)
  {
      mi_db_error_raise(NULL, MI_EXCEPTION, "AM_EOT_Reg: mi_register_callback failed!");
      return MI_ERROR;
  }

  memset( payload , 0 , 1024 );
  strcat( payload , "i," );
/*  mi_string *srvrName = mi_tab_server_name( tableDesc ); 
  strcat( payload , srvrName ); */
  mi_char hostname[HOST_NAME_MAX];
  gethostname(hostname , HOST_NAME_MAX);
  strcat( payload , hostname );
  strcat( payload , "," );

  mi_string *dbName = mi_tab_database_name( tableDesc );
  strcat( payload , dbName );
  strcat( payload , "," );

  mi_string *tabName = mi_tab_table_name( tableDesc );
  strcat( payload , tabName );
  strcat( payload , "," );
  rowToCSV( row , strchr( payload , 0 ) );

  ISSDEBUG(syslog( LOG_INFO, "Function %s: topic is %s.\n"  , __FUNCTION__ , mqtt->serverInfo->topic );)
  *endxact_payload = xact_payload_add( *endxact_payload , xact_payload_new( payload , mqtt , mqtt->serverInfo->topic ) );
  ISSDEBUG(syslog( LOG_INFO, "Function %s: topic is %s.\n"  , __FUNCTION__ , mqtt->serverInfo->topic );)

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
  return MI_OK;
}

mi_integer am_update( MI_AM_TABLE_DESC *tableDesc,
                      MI_ROW *oldRow,
                      MI_AM_ROWID_DESC *oldridDesc,
                      MI_ROW *newRow,
                      MI_AM_ROWID_DESC *newridDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Begin %s:\n" , __FUNCTION__ );)

  if( indexList == NULL ) return MI_OK;
  if( mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM ) != MI_OK )
  {
    ISSDEBUG(syslog( LOG_INFO, "Function %s: Error locking memory...\n" , __FUNCTION__ );)
    return MI_OK;
  }

  ISS_Index *index;
  ISS_MQTTSettings *mqtt;
  char *payload;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: Updating row of table...\n" , __FUNCTION__ );)
  if( ( index = getIndex( tableDesc ) ) == NULL ||
      ( mqtt = getMQTTClient( index ) ) == NULL ||
      ( payload = (char*)mi_dalloc( sizeof( char ) * 1024 , PER_TRANSACTION ) ) == NULL )
  {
    mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
    return MI_OK;
  }

  /* callback function prototype */
  MI_CALLBACK_HANDLE      *cback=NULL;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: Registering callback routine.\n" , __FUNCTION__ );)

  MI_CALLBACK_STATUS MI_PROC_CALLBACK am_eot_cb(MI_EVENT_TYPE type,MI_CONNECTION *conn,void *server_data,void *user_data);
  /* Register the MI_EVENT_COMMIT_ABORT callback handler. */
  cback = mi_register_callback (
        NULL,                  /* register on NULL conn handle  */
        MI_EVENT_COMMIT_ABORT, /* event = end of transaction    */
        am_eot_cb,             /* function server will dispatch */
        NULL,      /* user buffer */
        NULL);                 /* not used */

  if(cback == (MI_CALLBACK_HANDLE *) NULL)
  {
      mi_db_error_raise(NULL, MI_EXCEPTION, "AM_EOT_Reg: mi_register_callback failed!");
      return MI_ERROR;
  }

  mi_string *tabName = mi_tab_table_name( tableDesc );
  mi_string *dbName = mi_tab_database_name( tableDesc );
/*  mi_string *srvrName = mi_tab_server_name( tableDesc ); */

  mi_char hostname[HOST_NAME_MAX];
  gethostname(hostname , HOST_NAME_MAX);

  strcat( payload , hostname );
  memset( payload , 0 , 1024 );
  strcat( payload , "u," );
  /* strcat( payload , srvrName ); */
  strcat( payload , hostname );
  strcat( payload , "," );
  strcat( payload , dbName );
  strcat( payload , "," );
  strcat( payload , tabName );
  strcat( payload , "," );
  rowToCSV( newRow , strchr( payload , 0 ) );
  strcat( payload , "\nu," );
  /* strcat( payload , srvrName ); */
  strcat( payload , hostname );
  strcat( payload , "," );
  strcat( payload , dbName );
  strcat( payload , "," );
  strcat( payload , tabName );
  strcat( payload , "," );
  rowToCSV( oldRow , strchr( payload , 0 ) );

  *endxact_payload = xact_payload_add( *endxact_payload , xact_payload_new( payload , mqtt , mqtt->serverInfo->topic ) );

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
  return MI_OK;
}

mi_integer am_delete( MI_AM_TABLE_DESC *tableDesc, MI_ROW *row, MI_AM_ROWID_DESC *ridDesc )
{
  ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
  ISSDEBUG(syslog( LOG_INFO, "Begin %s:\n" , __FUNCTION__ );)

  if( indexList == NULL ) return MI_OK;
  if( mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM ) != MI_OK )
  {
    ISSDEBUG(syslog( LOG_INFO, "Function %s: Error locking memory...\n" , __FUNCTION__ );)
    return MI_OK;
  }

  ISS_Index *index;
  ISS_MQTTSettings *mqtt;
  char *payload;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: Deleting row from table...\n" , __FUNCTION__ );)
  if( ( index = getIndex( tableDesc ) ) == NULL ||
      ( mqtt = getMQTTClient( index ) ) == NULL ||
      ( payload = (char*)mi_dalloc( sizeof( char ) * 1024 , PER_TRANSACTION ) ) == NULL )
  {
    mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
    return MI_OK;
  }

  /* callback function prototype */
  MI_CALLBACK_HANDLE      *cback=NULL;

  ISSDEBUG(syslog( LOG_INFO, "Function %s: Registering callback routine.\n" , __FUNCTION__ );)

  MI_CALLBACK_STATUS MI_PROC_CALLBACK am_eot_cb(MI_EVENT_TYPE type,MI_CONNECTION *conn,void *server_data,void *user_data);
  /* Register the MI_EVENT_COMMIT_ABORT callback handler. */
  cback = mi_register_callback (
        NULL,                  /* register on NULL conn handle  */
        MI_EVENT_COMMIT_ABORT, /* event = end of transaction    */
        am_eot_cb,             /* function server will dispatch */
        NULL,      /* user buffer */
        NULL);                 /* not used */

  if(cback == (MI_CALLBACK_HANDLE *) NULL)
  {
      mi_db_error_raise(NULL, MI_EXCEPTION, "AM_EOT_Reg: mi_register_callback failed!");
      return MI_ERROR;
  }

  mi_string *tabName = mi_tab_table_name( tableDesc );
  memset( payload , 0 , 1024 );
  strcat( payload , "d," );
  /* mi_string *srvrName = mi_tab_server_name( tableDesc );
  strcat( payload , srvrName ); */
  mi_char hostname[HOST_NAME_MAX];
  gethostname(hostname , HOST_NAME_MAX);
  strcat( payload , hostname );
  strcat( payload , "," );
  mi_string *dbName = mi_tab_database_name( tableDesc );
  strcat( payload , dbName );
  strcat( payload , "," );
  strcat( payload , tabName );
  strcat( payload , "," );
  rowToCSV( row , strchr( payload , 0 ) );

  *endxact_payload = xact_payload_add( *endxact_payload , xact_payload_new( payload , mqtt , mqtt->serverInfo->topic ) );

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
  return MI_OK;
}

mi_integer am_beginscan( MI_AM_SCAN_DESC *scanDesc )
{
  return MI_OK;
}

mi_integer am_getnext( MI_AM_SCAN_DESC *scanDesc, MI_ROW **row, MI_AM_ROWID_DESC *ridDesc )
{
  return MI_OK;
}

mi_integer am_endscan( MI_AM_SCAN_DESC *scanDesc )
{
  return MI_OK;
}

mi_real* am_scancost( MI_AM_TABLE_DESC *tableDesc, MI_AM_QUAL_DESC *qualDesc )
{
  if( !constantScanCost )
  {
    constantScanCost = (mi_real*)mi_dalloc( sizeof( mi_real ) , PER_SYSTEM );
    *constantScanCost = -1;
  }

  return constantScanCost;
}

mi_integer am_truncate( MI_AM_TABLE_DESC *tableDesc )
{
  return MI_OK;
}

MI_CALLBACK_STATUS am_eot_cb (MI_EVENT_TYPE type, MI_CONNECTION *conn, void *server_info, void *user_data)
{

   MI_TRANSITION_TYPE      xact_type;

   ISSDEBUG(openlog( "InformixSocketStream" , 0, LOG_USER );)
   ISSDEBUG(syslog( LOG_INFO, "Function %s: End-Of-Transaction callback routine.\n" , __FUNCTION__ );)

   if(type != MI_EVENT_COMMIT_ABORT)
   {
      ISSDEBUG(syslog( LOG_INFO,"Function %s: EOT Callback called with inappropriate event %d\n", __FUNCTION__ , type);)
      return MI_CB_CONTINUE;
   }

    /* =========== Find out how the transaction ended =========== */
    xact_type = mi_transition_type(server_info);
    switch (xact_type)
    {
    case MI_ABORT_END:
         ISSDEBUG(syslog( LOG_INFO, "Function %s: Transaction being rolledback.\n"  , __FUNCTION__ );)
         break;
    case MI_NORMAL_END:
         ISSDEBUG(syslog( LOG_INFO,"Function %s: Transaction complete.\n" , __FUNCTION__ );)
         if( endxact_payload == NULL ) return MI_CB_CONTINUE;
       
         ISSDEBUG(syslog( LOG_INFO, "Function %s: Publishing transaction info.\n"  , __FUNCTION__ );)
         MqttPublish publish;
         memset( &publish , 0 , sizeof( MqttPublish ) );

/*
 * NOTE: The order of transactions in a begin work/commit work block is in reverse order on the linked list.
 * This will cause problems if there is an insert of a row and that same row is updated inside of the same
 * transaction iblock as the update will be listed before the insert.
 * As such we need to create a temporary linked list in the opposite order.
*/
         ENDXACT_PAYLOAD *current = NULL;
         ENDXACT_PAYLOAD *reverse = *endxact_payload;
         while( reverse != NULL )
         {
            if ( current == NULL )
            {
                current=reverse;
                current->next=NULL;
            } 
            else
            {
               current->next=reverse;
            }
            reverse = reverse->next;
         }

         while( current != NULL )
         {
            ISSDEBUG(syslog( LOG_INFO, "Function %s: Payload found.\n"  , __FUNCTION__ );)
            ISSDEBUG(syslog( LOG_INFO, "Function %s: Publish topic is %s.\n"  , __FUNCTION__ , current->topic );)
            ISSDEBUG(syslog( LOG_INFO, "Function %s: Publish payload is %s.\n"  , __FUNCTION__ , current->payload );)
            ISSDEBUG(syslog( LOG_INFO, "Function %s: nqtt->serverInfo->topic is %s.\n"  , __FUNCTION__ , current->mqtt->serverInfo->topic );)
            publish.retain = 0;
            publish.qos = 0;
            publish.duplicate = 0;
            publish.topic_name = current->topic;
            publish.buffer = (byte*)current->payload;
            publish.total_len = (word16)strlen( current->payload );
            publish.packet_id = mqttGetNextPacketID();
            MqttClient_Publish( current->mqtt->client, &publish );
            ISSDEBUG(syslog( LOG_INFO, "Function %s: Published payload.\n"  , __FUNCTION__ );)
            current = current->next;
            ISSDEBUG(syslog( LOG_INFO, "Function %s: Checking for more payload.\n"  , __FUNCTION__ );)
         }
         break;
    default:
         ISSDEBUG(syslog( LOG_INFO, "Function %s: Unknown transaction state.\n"  , __FUNCTION__ );)
         break;
    }

    *endxact_payload = NULL;
    ISSDEBUG(syslog( LOG_INFO, "Function %s: Exiting routine.\n"  , __FUNCTION__ );)
    return MI_CB_CONTINUE;
}

