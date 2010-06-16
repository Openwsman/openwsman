// loading.java
// 
// simple test for loading the bindings
// 
public class loading_jwsman {
  static {
    try {
	    String p = System.getProperty("java.library.path");
	    p = p + ":../../../build/bindings/java";
	    p = p + ":../.libs";
	    System.setProperty("java.library.path", p);
		    
	    System.out.println(p);
	    System.loadLibrary("jwsman");
    } catch (UnsatisfiedLinkError e) {
	    System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
	    System.exit(1);
    }
  }

  public static void main(String argv[]) {
	  System.out.println("jswsman loaded");
  }
}

