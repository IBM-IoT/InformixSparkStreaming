package com.ibm;

import tachyon.TachyonURI;
import tachyon.client.OutStream;
import tachyon.client.TachyonFS;
import tachyon.client.TachyonFile;
import tachyon.client.WriteType;

import java.lang.*;
import java.sql.SQLData;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.file.FileAlreadyExistsException;


public class TachyonVtiInterface {
    private static TachyonFS tachyonClient;
    private static TachyonURI mMasterLocation;
    private static TachyonURI mFilePath;
    private static WriteType mWriteType;


    public static void main(String[] args) {
       // System.out.println("Tachyon VTI Prototype");

        //TachyonVtiInterface tachyonVtiInterface = new TachyonVtiInterface();


        //String test = "test_table";
        //mMasterLocation = new TachyonURI("tachyon://127.0.0.1:19998/"); // TODO How would this get passed in from Informix?

        // Test our UDR functions.
        //tachyonVtiInterface.connectToTachyon(test);
        //tachyonVtiInterface.createTableInTachyon(test);
        //tachyonVtiInterface.deleteTableInTachyon(test);
        //tachyonVtiInterface.disconnectFromTachyon(test);
    }

    public static int connectToTachyon(Pointer MI_AM_TABLE_DESC){
/*        try {
            tachyonClient = TachyonFS.get(mMasterLocation);
        } catch (IOException e) {
            e.printStackTrace();
            return 1;
        }*/
        return 0;
    }

    public static int disconnectFromTachyon(Pointer MI_AM_TABLE_DESC){
/*        try {
            tachyonClient.close();
        } catch (IOException e) {
            e.printStackTrace();
            return 1;
        }*/
        return 0;
    }

    public static int createTableInTachyon(Pointer MI_AM_TABLE_DESC){

/*        mFilePath = new TachyonURI("/" + "test_ken_4242");
        try {

            tachyonClient.createFile(mFilePath);
        }
        catch (FileAlreadyExistsException e){
            System.out.println("Table already exists. Not doing anything.");
        } catch (IOException e) {
            e.printStackTrace();
            return 1;
        }

        ByteBuffer buf = ByteBuffer.allocate(4);
        buf.putChar('\u0000');
        try {
            TachyonFile file = tachyonClient.getFile(mFilePath);
            mWriteType = mWriteType.valueOf("MUST_CACHE");
            OutStream fileOutStream = file.getOutStream(mWriteType);
            fileOutStream.write(buf.array());
            fileOutStream.close();
        }
        catch (Exception e){
            e.printStackTrace();
            return 1;
        }*/
        return 0;
    }

    public static int deleteTableInTachyon(Pointer MI_AM_TABLE_DESC) {
//        mFilePath = new TachyonURI("/" + MI_AM_TABLE_DESC);
//        try {
//            tachyonClient.delete(mFilePath, false);
//        } catch (IOException e) {
//            e.printStackTrace();
//            return 1;
//        }
        return 0;
    }

    public static int getScanCost(String MI_AM_TABLE_DESC, String MI_AM_QUAL_DESC){
        return 100;
    }

    public static int beginScanTachyon(String MI_AM_SCAN_DESC){
        return 0;
    }

    public static int getNextTachyon(String MI_AM_SCAN_DESC, String MI_ROW, int mi_integer){
        return 0;
    }

    public static int endScanTachyon(String MI_AM_SCAN_DESC){
        return 0;
    }

    public static int insertTachyon(String MI_AM_TABLE_DESC, String MI_ROW, int mi_integer){
        return 0;
    }

    public static int getByte(String MI_AM_TABLE_DESC){
        return 0;
    }



}


