#include <mi.h>
#include <miami.h>
#include <sqltypes.h>

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

#include "MQTTClient.h"

#define AMPARAM_TOKEN_DELIMITERS " =,"

mi_real *constantScanCost = NULL;

typedef struct _ISS_LinkedList {
  void *payload;
  struct _ISS_LinkedList *next;
} ISS_LinkedList;

typedef struct {
  MQTTClient client;
  char *serverURI;
  int indexCount;
} ISS_MQTTSettings;

typedef struct {
  char *mqttTopic, *indexName;
  ISS_MQTTSettings *mqttSettings;
} ISS_Index;

ISS_LinkedList *indexList = 0, *mqttList = 0;

ISS_LinkedList* ISS_LinkedList_remove( ISS_LinkedList *list, ISS_LinkedList *link )
{
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

mi_integer getTableName( mi_string *indexName, char *dest )
{
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

mi_integer getMQTTServerURI( MI_AM_TABLE_DESC *tableDesc, char *serverURI )
{
  mi_string *params = mi_tab_amparam( tableDesc );
  syslog( LOG_INFO, "AM Params: %s\n", params );

  if( params )
  {
    char serverHost[128], serverPort[6];
    char *paramName = strtok( params, AMPARAM_TOKEN_DELIMITERS );
    char *paramValue = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
    while( paramName )
    {
      if( !paramValue )
      {
        syslog( LOG_INFO, "Missing param value for: %s\n", paramName );
        return MI_ERROR;
      }

      if( strcmp( paramName , "host" ) == 0 )
      {
        int len = strlen( paramValue );
        if( len > 127 ) return MI_ERROR;

        memcpy( serverHost , paramValue , len );
        serverHost[ len ] = 0;
      }
      else if( strcmp( paramName , "port" ) == 0 )
      {
        int len = strlen( paramValue );
        if( len > 5 ) return MI_ERROR;

        memcpy( serverPort , paramValue , len );
        serverPort[ len ] = 0;
      }
      else
      {
        syslog( LOG_INFO, "Unknown parameter name: %s\n", paramName );
        return MI_ERROR;
      }

      paramName = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
      paramValue = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
    }

    sprintf( serverURI , "tcp://%s:%s" , serverHost ,serverPort );
    syslog( LOG_INFO, "MQTT Server URI: %s\n" , serverURI );
  }
  else
  {
    syslog( LOG_INFO, "No parameters supplied.\n" );
    return MI_ERROR;
  }

  return MI_OK;
}

ISS_MQTTSettings* getMQTTSettings( char *serverURI )
{
  ISS_MQTTSettings *settings = NULL;
  ISS_LinkedList *current = mqttList;
  while( current != NULL )
  {
    settings = (ISS_MQTTSettings*)current->payload;
    if( strcmp( settings->serverURI , serverURI ) == 0 ) break;
    settings = NULL;
    current = current->next;
  }

  if( settings == NULL )
  {
    settings = (ISS_MQTTSettings*)mi_dalloc( sizeof(ISS_MQTTSettings) , PER_SYSTEM );
    settings->client = NULL;
    settings->serverURI = serverURI;
    settings->indexCount = 0;

    ISS_LinkedList *newLink = (ISS_LinkedList*)mi_dalloc( sizeof(ISS_LinkedList*) , PER_SYSTEM );
    newLink->payload = settings;
    mqttList = ISS_LinkedList_add( mqttList , newLink );
  }

  return settings;
}

void removeMQTTSettings( ISS_MQTTSettings *target )
{
    ISS_MQTTSettings *settings = NULL;
    ISS_LinkedList *current = mqttList;
    while( current != NULL )
    {
      settings = (ISS_MQTTSettings*)current->payload;
      if( settings == target ) break;
      settings = NULL;
      current = current->next;
    }

    if( settings != NULL )
    {
      mqttList = ISS_LinkedList_remove( mqttList , current );
      mi_free( current );

      syslog( LOG_INFO, "Removing MQTT Settings for: %s\n", settings->serverURI );

      if( settings->client )
      {
        if( MQTTClient_isConnected( settings->client ) ) MQTTClient_disconnect( settings->client , 1 );
        MQTTClient_destroy( &settings->client );
      }
      mi_free( settings->serverURI );
      mi_free( settings );
    }
}

mi_integer connectMQTTClient( ISS_MQTTSettings *mqttSettings )
{
  if( !mqttSettings->client || !MQTTClient_isConnected( mqttSettings->client ) )
  {
    MQTTClient_connectOptions connOpts = MQTTClient_connectOptions_initializer;
    connOpts.keepAliveInterval = 20;
    connOpts.cleansession = 1;
    connOpts.connectTimeout = 1;

    if( !mqttSettings->client )
    {
      syslog( LOG_INFO, "Creating new MQTT Client...\n" );
      int rc = MQTTClient_create( &mqttSettings->client , mqttSettings->serverURI , "IfxSparkStream" , MQTTCLIENT_PERSISTENCE_NONE , 0 );
      if( rc != MQTTCLIENT_SUCCESS )
      {
        syslog( LOG_INFO, "MQTT Client create error code: %d\n" , rc );
        return MI_ERROR;
      }
    }

    int rc = MQTTClient_connect( mqttSettings->client , &connOpts );
    if( rc != MQTTCLIENT_SUCCESS )
    {
      syslog( LOG_INFO, "Unable to connect to MQTT server. Error code: %d\n" , rc );
      return MI_ERROR;
    }

    syslog( LOG_INFO, "MQTT Client connected\n" );
  }

  return MI_OK;
}

ISS_Index* getIndex( MI_AM_TABLE_DESC *tableDesc )
{
  mi_string *indexName = mi_tab_name( tableDesc );
  ISS_LinkedList *current = indexList;
  ISS_Index *index = NULL;
  while( current != NULL )
  {
    index = (ISS_Index*)current->payload;
    if( strcmp( index->indexName , indexName ) == 0 ) break;
    index = NULL;
    current = current->next;
  }

  if( index == NULL )
  {
    int indexNameLen = strlen( indexName );

    index = (ISS_Index*)mi_dalloc( sizeof(ISS_Index) , PER_SYSTEM );
    index->mqttTopic = (char*)mi_dalloc( sizeof(char) * 256 , PER_SYSTEM );
    index->indexName = (char*)mi_dalloc( sizeof(char) * ( indexNameLen + 1 ) , PER_SYSTEM );
    memset( index->mqttTopic , 0 , 256 );
    memcpy( index->indexName , indexName , indexNameLen );
    index->indexName[ indexNameLen ] = 0;
    getTableName( indexName , index->mqttTopic );

    char *mqttServerURI = (char*)mi_dalloc( sizeof(char) * 256 , PER_SYSTEM );
    memset( mqttServerURI , 0 , 256 );
    if( getMQTTServerURI( tableDesc , mqttServerURI ) == MI_ERROR ) return NULL;
    index->mqttSettings = getMQTTSettings( mqttServerURI );
    index->mqttSettings->indexCount++;

    ISS_LinkedList *newLink = (ISS_LinkedList*)mi_dalloc( sizeof(ISS_LinkedList) , PER_SYSTEM );
    newLink->payload = index;
    indexList = ISS_LinkedList_add( indexList , newLink );

    syslog( LOG_INFO, "Added index: %s\n" , ((ISS_Index*)newLink->payload)->indexName );
  }

  if( connectMQTTClient( index->mqttSettings ) != MI_OK ) index = NULL;

  return index;
}

void removeIndex( mi_string *indexName )
{
  ISS_Index *index = NULL;
  ISS_LinkedList *current = indexList;
  while( current != NULL )
  {
    index = (ISS_Index*)current->payload;
    if( strcmp( index->indexName , indexName ) == 0 ) break;
    index = NULL;
    current = current->next;
  }

  if( index != NULL )
  {
    indexList = ISS_LinkedList_remove( indexList , current );
    mi_free( current );

    syslog( LOG_INFO, "Removed index: %s\n", index->indexName );

    mi_free( index->mqttTopic );
    mi_free( index->indexName );
    index->mqttSettings->indexCount--;
    if( index->mqttSettings->indexCount <= 0 )
    {
      removeMQTTSettings( index->mqttSettings );
    }
    mi_free( index );
  }
}

void columnValueToString( MI_ROW *row, mi_integer index, char *dest )
{
  MI_DATUM valueBuffer;
  mi_integer valueLen = 0;
  mi_string *stringValue;

  // TODO: Make sure typecasting MI_TYPEID to mi_integer is safe
  mi_integer columnTypeID = *(mi_integer*)mi_column_type_id( (MI_ROW_DESC*)row , index );
  // syslog( LOG_INFO, "Column: %d, Type ID: %d\n" , index , *(mi_integer*)columnTypeID );

  mi_integer valueType = mi_value( row ,  index , &valueBuffer , &valueLen );
  // syslog( LOG_INFO, "Value len: %d\n", valueLen );

  if( valueType == MI_NORMAL_VALUE )
  {
    switch( columnTypeID & TYPEIDMASK )
    {
      case SQLINT:
        sprintf( dest, "%d", *(mi_integer*)(&valueBuffer) );
        break;
      case SQLMONEY:
        stringValue = mi_money_to_string( (mi_money*)valueBuffer );
        strcat( dest , stringValue );
        mi_free( stringValue );
        break;
      case SQLVCHAR:
        stringValue = mi_lvarchar_to_string( (mi_lvarchar*)valueBuffer );
        strcat( dest , stringValue );
        mi_free( stringValue );
        break;
      default:
        syslog( LOG_INFO, "Unknown column type ID: %d\n", columnTypeID );
        break;
    }
  }
  else
  {
    syslog( LOG_INFO, "Unknown value type\n" );
  }
}

void rowToCSV( MI_ROW *row, char *dest )
{
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
  openlog( "InformixSparkStream" , 0, LOG_USER );
  mi_string *indexName = mi_tab_name( tableDesc );
  syslog( LOG_INFO, "Index created: %s\n" , indexName );

  return MI_OK;
}

mi_integer am_drop( MI_AM_TABLE_DESC *tableDesc )
{
  openlog( "InformixSparkStream" , 0, LOG_USER );
  mi_string *indexName = mi_tab_name( tableDesc );

  removeIndex( indexName );
  syslog( LOG_INFO, "Index dropped: %s\n" , indexName );

  return MI_OK;
}

mi_integer am_open( MI_AM_TABLE_DESC *tableDesc )
{
  return MI_OK;
}

mi_integer am_close( MI_AM_TABLE_DESC *tableDesc )
{
  return MI_OK;
}

mi_integer am_insert( MI_AM_TABLE_DESC *tableDesc, MI_ROW *row, MI_AM_ROWID_DESC *ridDesc )
{
  ISS_Index *index = getIndex( tableDesc );
  if( !index ) return MI_OK;

  openlog( "InformixSparkStream" , 0, LOG_USER );

  MQTTClient_message msg = MQTTClient_message_initializer;
  msg.payload = mi_alloc(sizeof(char) * 2048);
  if( msg.payload == NULL ) return MI_ERROR;
  memset( msg.payload , 0 , 2048 );
  strcat( (char*)msg.payload , "i," );
  rowToCSV( row , strchr( (char*)msg.payload , 0 ) );
  msg.payloadlen = strlen( msg.payload );
  msg.qos = 0;
  msg.retained = 0;
  MQTTClient_publishMessage( index->mqttSettings->client , index->mqttTopic , &msg , NULL );

  // syslog( LOG_INFO, "Topic: %s\n", mqttTopic );
  // syslog( LOG_INFO, "Payload: %s\n", (char*)msg.payload );
  mi_free( msg.payload );

  return MI_OK;
}

mi_integer am_update( MI_AM_TABLE_DESC *tableDesc,
                      MI_ROW *oldRow,
                      MI_AM_ROWID_DESC *oldridDesc,
                      MI_ROW *newRow,
                      MI_AM_ROWID_DESC *newridDesc )
{
  ISS_Index *index = getIndex( tableDesc );
  if( !index ) return MI_OK;

  openlog( "InformixSparkStream" , 0, LOG_USER );

  MQTTClient_message msg = MQTTClient_message_initializer;
  msg.payload = mi_alloc(sizeof(char) * 2048);
  if( msg.payload == NULL ) return MI_ERROR;
  memset( msg.payload , 0 , 2048 );
  strcat( (char*)msg.payload , "u," );
  rowToCSV( newRow , strchr( (char*)msg.payload , 0 ) );
  strcat( (char*)msg.payload , "\nu," );
  rowToCSV( oldRow , strchr( (char*)msg.payload , 0 ) );

  msg.payloadlen = strlen( msg.payload );
  msg.qos = 0;
  msg.retained = 0;
  MQTTClient_publishMessage( index->mqttSettings->client , index->mqttTopic , &msg , NULL );

  // syslog( LOG_INFO, "Topic: %s\n", mqttTopic );
  // syslog( LOG_INFO, "Payload: %s\n", (char*)msg.payload );
  mi_free( msg.payload );

  return MI_OK;
}

mi_integer am_delete( MI_AM_TABLE_DESC *tableDesc, MI_ROW *row, MI_AM_ROWID_DESC *ridDesc )
{
  ISS_Index *index = getIndex( tableDesc );
  if( !index ) return MI_OK;

  openlog( "InformixSparkStream" , 0, LOG_USER );

  MQTTClient_message msg = MQTTClient_message_initializer;
  msg.payload = mi_alloc(sizeof(char) * 2048);
  if( msg.payload == NULL ) return MI_ERROR;
  memset( msg.payload , 0 , 2048 );
  strcat( (char*)msg.payload , "d," );
  rowToCSV( row , strchr( (char*)msg.payload , 0 ) );
  msg.payloadlen = strlen( msg.payload );
  msg.qos = 0;
  msg.retained = 0;
  MQTTClient_publishMessage( index->mqttSettings->client , index->mqttTopic , &msg , NULL );

  // syslog( LOG_INFO, "Topic: %s\n", mqttTopic );
  // syslog( LOG_INFO, "Payload: %s\n", (char*)msg.payload );
  mi_free( msg.payload );

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

