import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.XmlDoc;
import org.openwsman.OpenWSManConstants;
import org.openwsman.XmlNode;

public class cim_enumerate_registered_profiles {

	private static final String URI = OpenWSManConstants.XML_NS_CIM_CLASS + "/CIM_RegisteredProfile";

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Client client = new Client(
				"http://wsman:secret@localhost:5985/wsman");
		ClientOptions options = new ClientOptions();
//		options.set_dump_request();
		options.add_selector(OpenWSManConstants.CIM_NAMESPACE_SELECTOR,
				"root/interop");
		client.transport().set_auth_method(OpenWSManConstants.BASIC_AUTH_STR);

		XmlDoc result = client.enumerate(options, null, URI);
		if (result == null || result.isFault())
			System.err.println("Enumeration failed: "
					+ ((result != null) ? result.fault().reason() : "?"));
		else {
			String context = result.context();
			while (context != null) {
				result = client.pull(options, null, URI, context);
				if (result == null || result.isFault())	 {
					System.err.println("Pull failed: " +
							((result != null) ? result.fault().reason() : "?"));
					context = null;
					continue;
				}

				XmlNode id = result.root().find(null,"InstanceID", 1);
				XmlNode name = result.root().find(null,"RegisteredName", 1);
				
				System.out.println(id + " (" + name	+ ")");
				context = result.context();
			}
		}
	}
}
