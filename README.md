This demo is a modification to the originl InforixSparkStreaming. The changes here were needed to allow for proper transaction processing. As such much of the original code is still in use. Below is the original text of the README file. To get started with this version read the setup_vii_demo.txt file.

### Changes in this version
-- Added transactional support
-- Modifications to only publish to MQTT when a transaction is successfully completed
-- Removed code that set the MQTT topic to the table name
-- Added ability to set the MQTT topic upon index creation
-- Added hostname, database name and table name to output
-- Changed demo to work with stores_demo database

### Original text
# Informix Socket Streaming

Informix Socket Streaming is an extension of Informix that allows data to be streamed out of the database as soon as it is inserted, updated, or deleted.

The protocol currently used to stream the changes is MQTT v3.1.1 (older versions not supported!). This extension is able to stream data to any MQTT broker where it can be processed or passed on to subscribing clients for processing.

#### Value Proposition
[This presentation](http://www.slideshare.net/deepind/informix-mqtt-streaming) provides the background and the value proposition of this solution. Along with the use cases across multiple industry sectors, understand the need for fast streaming of data from an Informix database. The presentation also shows the architecture and implementation of a demo based on Internet of Things (IoT) sensor data for the healthcare industry.

## Build and Install

#### Pre-requisites
* autoconf, automake, libtool
* gcc
* git

#### Build/Install
To build and install this extension run `setup.sh` in the main directory.
You need to be logged in as an Informix user and the environment
variable `$INFORMIXDIR` needs to be set properly

This script should retrieve the required libraries using Git, build them,
build the extension and install it in `$INFORMIXDIR/extend/`

## Now what?
To use this extension, you will need to register the extension and its functions with a database.
Then, you can configure it to stream data from any table as rows get added, deleted, or modified.

To learn more about how to use the extension, see the example SQL files in `examples/`

The following column data types are currently supported:
* CHAR
* VARCHAR
* SMALLINT, INTEGER
* SMALLFLOAT, FLOAT
* DECIMAL
* SERIAL
* MONEY
* DATE ... DATETIME YEAR TO FRACTION(5)
