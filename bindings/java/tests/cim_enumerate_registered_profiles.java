import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.XmlDoc;
import org.openwsman.OpenWSManConstants;

public class cim_enumerate_registered_profiles {

	private static final String URI = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_RegisteredProfile";

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Client client = new Client(
				"http://wsman:secret@linux-3qev.localnet:5985/wsman");
		ClientOptions options = new ClientOptions();
		options.set_dump_request();
		options.add_selector(OpenWSManConstants.CIM_NAMESPACE_SELECTOR,
				"root/interop");
		client.transport().set_auth_method(OpenWSManConstants.BASIC_AUTH_STR);

		XmlDoc result = client.enumerate(options, null, URI);
		if (result == null || result.isFault())
			System.err.println("Enumeration failed: "
					+ ((result != null) ? result.fault().detail() : "?"));
		else {
			String context = result.context();
			while (context != null) {
				System.out.println("Context: " + context);
				result = client.pull(options, null, URI, context);
				if (result == null || result.isFault())	 {
					System.err.println("Pull failed: " +
							((result != null) ? result.fault().detail() : "?"));
					context = null;
					continue;
				}
				System.out.println(result.encode("UTF-8"));
				context = result.context();				
			}
		}
	}
}
