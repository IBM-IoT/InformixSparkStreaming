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
import java.util.concurrent.ConcurrentNavigableMap;


public class tachyon_vti_interface {
    private static final TachyonURI mMasterLocation = new TachyonURI("tachyon://127.0.0.1:19998/");

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
        } catch (IOException e) {
            e.printStackTrace();
        }
        return 0;
    }

    private static void createFile(TachyonFS tachyonClient) throws IOException {
        int fileId = tachyonClient.createFile("/table");
    }

}
