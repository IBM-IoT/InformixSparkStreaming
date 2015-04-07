package com.ibm;
import org.mapdb.*;
import tachyon.TachyonURI;
import tachyon.Constants;
import tachyon.TachyonURI;
import tachyon.Version;
import tachyon.client.OutStream;
import tachyon.client.TachyonByteBuffer;
import tachyon.client.TachyonFile;
import tachyon.client.TachyonFS;
import tachyon.client.WriteType;
import tachyon.util.CommonUtils;
import tachyon.client.*;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.FileAlreadyExistsException;
import java.util.concurrent.ConcurrentNavigableMap;


public class tachyon_vti_interface {
    private static final TachyonURI mMasterLocation = new TachyonURI("tachyon://127.0.0.1:19998/");
    private static final int mNumbers = 20;
    private static final TachyonURI mFilePath = new TachyonURI("/table");
    private static WriteType mWriteType;

    public static void main(String[] args) {
        System.out.println("Tachyon VTI Prototype");
        String test = "test";
        am_create(test);


    }

    public int am_open (String MI_AM_TABLE_DESC){
        return 0;
    }

    public int am_close (String MI_AM_TABLE_DESC){
        return 0;
    }

    public static int am_create (String MI_AM_TABLE_DESC){
        try {
            TachyonFS tachyonClient = TachyonFS.get(mMasterLocation);
            createFile(tachyonClient);
            writeFile(tachyonClient);
        }
        catch (FileAlreadyExistsException e){
            System.out.println("Table already exists. Not doing anything.");
        } catch (IOException e) {
            e.printStackTrace();
        }
        return 0;
    }

    private static void createFile(TachyonFS tachyonClient) throws IOException {
        try {
            int fileId = tachyonClient.createFile("/table4");
        }
        catch (FileAlreadyExistsException e) {
            System.out.println("Table already exists. Nothing to do.");
        }
        catch (Exception e){
            e.printStackTrace();
        }

    }

    private static void writeFile(TachyonFS tachyonClient) throws IOException {
        ByteBuffer buf = ByteBuffer.allocate(mNumbers * 4);
        buf.order(ByteOrder.nativeOrder());
        for (int k = 0; k < mNumbers; k ++) {
            buf.putInt(k);
        }
        buf.flip();
        buf.flip();
        long startTimeMs = CommonUtils.getCurrentMs();
        TachyonFile file = tachyonClient.getFile(mFilePath);
        mWriteType = mWriteType.valueOf("MUST_CACHE");
        OutStream os = file.getOutStream(mWriteType);
        os.write(buf.array());
        os.close();
    }

}
