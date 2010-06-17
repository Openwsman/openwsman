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
		if (doc == null || (doc.is_fault() != 0)) {
			System.out.println("Failed to identify");
			System.exit(1);
		}

		System.out.println("ProtocolVersion: "
				+ doc.root().find(jwsmanConstants.XML_NS_WSMAN_ID,
						"ProtocolVersion", 1).text());
		System.out.println("ProductVendor: "
				+ doc.root().find(jwsmanConstants.XML_NS_WSMAN_ID,
						"ProductVendor", 1).text());
		System.out.println("ProductVersion: "
				+ doc.root().find(jwsmanConstants.XML_NS_WSMAN_ID,
						"ProductVersion", 1).text());
	}
}
