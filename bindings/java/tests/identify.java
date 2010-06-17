//
// identify.java
// 
// Java test for WS-Identify
//

import org.openwsman.*;

public class identify {
  public static void main(String argv[]) {
	  Client c = new Client("http://wsman:secret@localhost:5985/wsman");

	  ClientOptions op = new ClientOptions();
	  op.set_dump_request();
	  c.transport().set_auth_method("basic");
	  XmlDoc doc = c.identify(op);
	  if (doc != null)
		  System.out.println(doc.encode("UTF-8"));
	  else
		  System.out.println("Failed to identify");
  }
}

