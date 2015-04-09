package com.ibm;

import tachyon.TachyonURI;
import tachyon.client.OutStream;
import tachyon.client.TachyonFS;
import tachyon.client.TachyonFile;
import tachyon.client.WriteType;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.file.FileAlreadyExistsException;

import org.mapdb.*;


public class TachyonVtiInterface {
    private static TachyonFS tachyonClient;
    private static TachyonURI mMasterLocation;
    private static TachyonURI mFilePath;
    private static WriteType mWriteType;


    public static void main(String[] args) {
        System.out.println("Tachyon VTI Prototype");

        TachyonVtiInterface tachyonVtiInterface = new TachyonVtiInterface();


        String test = "test_table";
        mMasterLocation = new TachyonURI("tachyon://127.0.0.1:19998/");

        tachyonVtiInterface.am_open(test);
        tachyonVtiInterface.am_create(test);
        tachyonVtiInterface.am_drop(test);
        tachyonVtiInterface.am_close(test);
    }

    public int am_open (String MI_AM_TABLE_DESC){
        try {
            tachyonClient = TachyonFS.get(mMasterLocation);
        } catch (IOException e) {
            e.printStackTrace();
            return 1;
        }
        return 0;
    }

    public int am_close (String MI_AM_TABLE_DESC){
        try {
            tachyonClient.close();
        } catch (IOException e) {
            e.printStackTrace();
            return 1;
        }
        return 0;
    }

    public int am_create (String MI_AM_TABLE_DESC){

        mFilePath = new TachyonURI("/" + MI_AM_TABLE_DESC);
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
        }
        return 0;
    }

    public int am_drop (String MI_AM_TABLE_DESC) {
        mFilePath = new TachyonURI("/" + MI_AM_TABLE_DESC);
        try {
            tachyonClient.delete(mFilePath, false);
        } catch (IOException e) {
            e.printStackTrace();
            return 1;
        }
        return 0;
    }

    public int am_scancost(String MI_AM_TABLE_DESC, String MI_AM_QUAL_DESC){
        return 100;
    }

    public int am_beginscan(String MI_AM_SCAN_DESC){
        return 0;
    }

    public int am_getnext(String MI_AM_SCAN_DESC, String MI_ROW, int mi_integer){
        return 0;
    }

    public int am_endscan(String MI_AM_SCAN_DESC){
        return 0;
    }

    public int am_insert(String MI_AM_TABLE_DESC){
        return 0;
    }

}
