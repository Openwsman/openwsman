/*
 * Base class for Java to properly initialize the shared library
 * 
 * Author: Jan Blunck  <jblunck@suse.de>
 * 
 */
package org.openwsman;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;

public class JWsManBase {
    static {
	InputStream inputStream = JWsManBase.class.getClassLoader().getResourceAsStream("jwsman.so");

	try {
	    File libraryFile = File.createTempFile("jwsman.so", null);
	    libraryFile.deleteOnExit();

	    FileOutputStream fileOutputStream = new FileOutputStream(libraryFile);
	    byte[] buffer = new byte[8192];
	    int bytesRead;
	    while((bytesRead = inputStream.read(buffer)) > 0){
		fileOutputStream.write(buffer, 0, bytesRead);
	    }
	    fileOutputStream.close();
	    inputStream.close();

	    try {
		System.load(libraryFile.getPath());
	    } catch (UnsatisfiedLinkError e) {
		System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
		System.exit(1);
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }
}
