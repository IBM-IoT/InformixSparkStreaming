package com.ibm;
import org.mapdb.*;

import java.util.concurrent.ConcurrentNavigableMap;


public class tachyon_vti_interface {

    public static void main(String[] args) {
        System.out.println("Test");



        DB db = DBMaker.newMemoryDB().make();

        ConcurrentNavigableMap treeMap = db.getTreeMap("map");
        treeMap.put("something","here");

        db.commit();
        db.close();

    }

    public int am_open (String MI_AM_TABLE_DESC){
        return 0;
    }

    public int am_close (String MI_AM_TABLE_DESC){
        return 0;
    }

    public int am_create (String MI_AM_TABLE_DESC){
        return 0;
    }

}
