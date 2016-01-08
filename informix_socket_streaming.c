#include <mi.h>
#include <miami.h>
#include <minmprot.h>
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

#include "mqttnet.h"

#define AMPARAM_TOKEN_DELIMITERS " =,"
#define INDEX_LIST_MEMNAME "ISSIndexList"
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

ISS_LinkedList **indexList = NULL;//, *mqttList = NULL;
int nextMQTTClientID = 0;

int mqttLastPacketID = 0;
word16 mqttGetNextPacketID()
{
  mqttLastPacketID = ( mqttLastPacketID >= MQTT_MAX_PACKET_ID ) ? 1 : mqttLastPacketID + 1;
  return mqttLastPacketID;
}

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

ISS_LinkedList* ISS_LinkedList_new( void *payload )
{
  ISS_LinkedList *newLink = (ISS_LinkedList*)mi_dalloc( sizeof(ISS_LinkedList) , PER_SYSTEM );
  newLink->payload = payload;
  newLink->next = NULL;
  return newLink;
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

ISS_ServerInfo* getMQTTServerInfo( MI_AM_TABLE_DESC *tableDesc )
{
  mi_string *params = mi_tab_amparam( tableDesc );
  syslog( LOG_INFO, "AM Params: %s\n", params );

  ISS_ServerInfo *info = NULL;

  if( params )
  {
    char *serverHost = NULL;
    int serverPort = 0;
    char *paramName = strtok( params, AMPARAM_TOKEN_DELIMITERS );
    char *paramValue = strtok( 0, AMPARAM_TOKEN_DELIMITERS );
    while( paramName )
    {
      if( !paramValue )
      {
        syslog( LOG_INFO, "Missing param value for: %s\n", paramName );
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
      else
      {
        syslog( LOG_INFO, "Unknown parameter name: %s\n", paramName );
        continue;
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

      syslog( LOG_INFO, "MQTT Server Info: %s:%d\n", info->host, info->port );
    }
    else if( serverHost ) mi_free( serverHost );
  }
  else
  {
    syslog( LOG_INFO, "No parameters supplied.\n" );
    return NULL;
  }

  return info;
}

ISS_MQTTSettings* getMQTTSettings( ISS_ServerInfo *serverInfo )
{
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
    if( settings != NULL )
    {
      syslog( LOG_INFO, "[%d] Removing MQTT Settings: %d,%s:%d\n", getpid(), settings->pid, settings->serverInfo->host, settings->serverInfo->port );

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

      syslog( LOG_INFO, "[%d] Client disconnected/destroyed...\n", getpid() );

      mi_free( settings );
    }
}

mi_integer connectMQTTClient( ISS_MQTTSettings *mqttSettings )
{
  if( !mqttSettings->client || ( mqttSettings->client->flags & MQTT_CLIENT_FLAG_IS_CONNECTED ) == 0 )
  {
    int rc;
    MqttConnect connect;

    if( !mqttSettings->client )
    {
      syslog( LOG_INFO , "[%d] MQTT Client does not exist.\n", getpid() );

      MqttNet *net = (MqttNet*)mi_dalloc( sizeof( MqttNet ) , PER_SYSTEM );
      rc = MqttClientNet_Init( net );
      if( rc != 0 )
      {
        syslog( LOG_INFO, "[%d] MQTT Net Init failed: %s\n", getpid(), MqttClient_ReturnCodeToString( rc ) );
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
        syslog( LOG_INFO, "[%d] MQTT Connect failed: %s\n", getpid(), MqttClient_ReturnCodeToString( rc ) );

        mi_free( txBuffer );
        mi_free( rxBuffer );
        mi_free( mqttSettings->client );
        mqttSettings->client = NULL;
        return MI_ERROR;
      }
    }
    else
    {
      syslog( LOG_INFO , "[%d] MQTT Client not connected.\n", getpid() );
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
      syslog( LOG_INFO, "[%d] MQTT Connect failed: %s\n", getpid(), MqttClient_ReturnCodeToString( rc ) );
      return MI_ERROR;
    }

    syslog( LOG_INFO, "[%d] MQTT Client connected\n", getpid() );
  }

  return MI_OK;
}

ISS_Index* getIndex( MI_AM_TABLE_DESC *tableDesc )
{
  if( indexList == NULL ) return NULL;

  mi_string *indexName = mi_tab_name( tableDesc );
  ISS_LinkedList *current = *indexList;
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
    ISS_ServerInfo *serverInfo = getMQTTServerInfo( tableDesc );
    if( serverInfo == NULL ) return NULL;

    current = *indexList;
    while( current != NULL )
    {
      index = (ISS_Index*)current->payload;
      if( strcmp( index->serverInfo->host , serverInfo->host ) == 0 && index->serverInfo->port == serverInfo->port )
      {
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

    syslog( LOG_INFO, "[%d] Added index: %s\n", getpid(), indexName );
  }

  return index;
}

ISS_MQTTSettings* getMQTTClient( ISS_Index *index )
{
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

    syslog( LOG_INFO, "Removing index: %s\n", index->indexName );

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

      syslog( LOG_INFO, "[%d] Removing MQTT Link...\n", getpid() );

      prevMQTT = currentMQTT;
      currentMQTT = currentMQTT->next;
      mi_free( prevMQTT );

      syslog( LOG_INFO, "[%d] Done.\n", getpid() );
    }

    index->serverInfo->refCount--;
    if( index->serverInfo->refCount <= 0 )
    {
      syslog( LOG_INFO, "[%d] Removing ServerInfo %s:%d\n", getpid(), index->serverInfo->host, index->serverInfo->port );
      mi_free( index->serverInfo->host );
      mi_free( index->serverInfo );
    }

    mi_free( index->mqttTopic );
    mi_free( index->indexName );
    mi_free( index );

    syslog( LOG_INFO, "[%d] Index removed.\n", getpid() );
  }

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
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
  if( indexList == NULL )
  {
    int rc = mi_named_get( INDEX_LIST_MEMNAME, PER_SYSTEM, (void**)&indexList );
    if( rc == MI_ERROR )
    {
      syslog( LOG_INFO, "Error getting indexList.\n" );
    }
    if( rc == MI_NO_SUCH_NAME )
    {
      rc = mi_named_alloc( sizeof( ISS_LinkedList* ), INDEX_LIST_MEMNAME, PER_SYSTEM, (void**)&indexList );
      if( rc == MI_ERROR )
      {
        syslog( LOG_INFO, "Error allocating indexList.\n" );
      }

      *indexList = NULL;
    }
    // indexList = (ISS_LinkedList**)mi_dalloc( sizeof( ISS_LinkedList* ), PER_SYSTEM );
    // *indexList = NULL;
  }

  return MI_OK;
}

mi_integer am_close( MI_AM_TABLE_DESC *tableDesc )
{
  return MI_OK;
}

mi_integer am_insert( MI_AM_TABLE_DESC *tableDesc, MI_ROW *row, MI_AM_ROWID_DESC *ridDesc )
{
  openlog( "InformixSparkStream" , 0, LOG_USER );

  if( indexList == NULL ) return MI_OK;
  if( mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM ) != MI_OK )
  {
    syslog( LOG_INFO, "Error locking memory...\n" );
    return MI_OK;
  }

  ISS_Index *index;
  ISS_MQTTSettings *mqtt;
  char *payload;

  if( ( index = getIndex( tableDesc ) ) == NULL ||
      ( mqtt = getMQTTClient( index ) ) == NULL ||
      ( payload = (char*)mi_alloc( sizeof( char ) * 1024 ) ) == NULL )
  {
    mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
    return MI_OK;
  }

  memset( payload , 0 , 1024 );
  strcat( payload , "i," );
  rowToCSV( row , strchr( payload , 0 ) );

  MqttPublish publish;
  memset( &publish , 0 , sizeof( MqttPublish ) );
  publish.retain = 0;
  publish.qos = 0;
  publish.duplicate = 0;
  publish.topic_name = index->mqttTopic;
  publish.buffer = (byte*)payload;
  publish.total_len = (word16)strlen( payload );
  publish.packet_id = mqttGetNextPacketID();

  MqttClient_Publish( mqtt->client, &publish );
  mi_free( payload );

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
  return MI_OK;
}

mi_integer am_update( MI_AM_TABLE_DESC *tableDesc,
                      MI_ROW *oldRow,
                      MI_AM_ROWID_DESC *oldridDesc,
                      MI_ROW *newRow,
                      MI_AM_ROWID_DESC *newridDesc )
{
  openlog( "InformixSparkStream" , 0, LOG_USER );

  if( indexList == NULL ) return MI_OK;
  if( mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM ) != MI_OK )
  {
    syslog( LOG_INFO, "Error locking memory...\n" );
    return MI_OK;
  }

  ISS_Index *index;
  ISS_MQTTSettings *mqtt;
  char *payload;

  if( ( index = getIndex( tableDesc ) ) == NULL ||
      ( mqtt = getMQTTClient( index ) ) == NULL ||
      ( payload = (char*)mi_alloc( sizeof( char ) * 1024 ) ) == NULL )
  {
    mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
    return MI_OK;
  }

  memset( payload , 0 , 1024 );
  strcat( payload , "u," );
  rowToCSV( newRow , strchr( payload , 0 ) );
  strcat( payload , "\nu," );
  rowToCSV( oldRow , strchr( payload , 0 ) );

  MqttPublish publish;
  memset( &publish , 0 , sizeof( MqttPublish ) );
  publish.retain = 0;
  publish.qos = 0;
  publish.duplicate = 0;
  publish.topic_name = index->mqttTopic;
  publish.buffer = (byte*)payload;
  publish.total_len = (word16)strlen( payload );
  publish.packet_id = mqttGetNextPacketID();

  MqttClient_Publish( mqtt->client, &publish );
  mi_free( payload );

  mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
  return MI_OK;
}

mi_integer am_delete( MI_AM_TABLE_DESC *tableDesc, MI_ROW *row, MI_AM_ROWID_DESC *ridDesc )
{
  openlog( "InformixSparkStream" , 0, LOG_USER );

  if( indexList == NULL ) return MI_OK;
  if( mi_lock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM ) != MI_OK )
  {
    syslog( LOG_INFO, "Error locking memory...\n" );
    return MI_OK;
  }

  ISS_Index *index;
  ISS_MQTTSettings *mqtt;
  char *payload;

  if( ( index = getIndex( tableDesc ) ) == NULL ||
      ( mqtt = getMQTTClient( index ) ) == NULL ||
      ( payload = (char*)mi_alloc( sizeof( char ) * 1024 ) ) == NULL )
  {
    mi_unlock_memory( INDEX_LIST_MEMNAME , PER_SYSTEM );
    return MI_OK;
  }

  memset( payload , 0 , 1024 );
  strcat( payload , "d," );
  rowToCSV( row , strchr( payload , 0 ) );

  MqttPublish publish;
  memset( &publish , 0 , sizeof( MqttPublish ) );
  publish.retain = 0;
  publish.qos = 0;
  publish.duplicate = 0;
  publish.topic_name = index->mqttTopic;
  publish.buffer = (byte*)payload;
  publish.total_len = (word16)strlen( payload );
  publish.packet_id = mqttGetNextPacketID();

  MqttClient_Publish( mqtt->client, &publish );
  mi_free( payload );

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

