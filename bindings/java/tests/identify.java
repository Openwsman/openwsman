public class test_identify {
  static {
    try {
	System.loadLibrary("jwsman");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
      System.exit(1);
    }
  }

  public static void main(String argv[]) {
	  Client c = new Client("http://wsman:secret@localhost:8889/wsman");

	  ClientOptions op = new ClientOptions();
	  WsXmlDoc doc = c.identify(op);
	  System.out.println(doc.dump("UTF-8"));
  }
}

