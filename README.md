# Informix Socket Streaming

Informix Socket Streaming is an extension of Informix that allows data to be streamed out of the database as soon as it is inserted, updated, or deleted.

The protocol currently used to stream the changes is MQTT v3.1.1 (older versions not supported!). This extension is able to stream data to any MQTT broker where it can be processed or passed on to subscribing clients for processing.

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

To learn how to use the extension, see the example SQL files in examples/