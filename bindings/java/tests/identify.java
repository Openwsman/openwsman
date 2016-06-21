//
// identify.java
// 
// Java test for WS-Identify
//

import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.OpenWSManConstants;
import org.openwsman.XmlDoc;

public class identify {

	public static void main(String argv[]) {
		Client c = new Client("http://wsman:secret@localhost:5985/wsman");
		ClientOptions op = new ClientOptions();
		op.set_dump_request();
		c.transport().set_auth_method(OpenWSManConstants.BASIC_AUTH_STR);

		XmlDoc doc = c.identify(op);
		if (doc == null || doc.isFault()) {
			System.err.println("Failed to identify: " + ((doc != null) ? doc.fault().reason() : "?"));
			System.exit(1);
		}

		System.out.println("ProtocolVersion: "
				+ doc.root().find(OpenWSManConstants.XML_NS_WSMAN_ID,
						OpenWSManConstants.WSMID_PROTOCOL_VERSION, 1));
		System.out.println("ProductVendor: "
				+ doc.root().find(OpenWSManConstants.XML_NS_WSMAN_ID,
						OpenWSManConstants.WSMID_PRODUCT_VENDOR, 1));
		System.out.println("ProductVersion: "
				+ doc.root().find(OpenWSManConstants.XML_NS_WSMAN_ID,
						OpenWSManConstants.WSMID_PRODUCT_VERSION, 1));
	}
}
