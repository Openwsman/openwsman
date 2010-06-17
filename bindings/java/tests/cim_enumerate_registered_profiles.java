import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.XmlDoc;
import org.openwsman.jwsmanConstants;

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
		System.err.println(jwsmanConstants.CIM_NAMESPACE_SELECTOR);
//		options.add_selector(jwsmanConstants.CIM_NAMESPACE_SELECTOR, "root/interop");
		options.add_selector("__cimnamespace", "root/interop");
		client.transport().set_auth_method("basic");

		System.err.println("Enumerate");
		XmlDoc result = client.enumerate(options, null, URI);
		if (result == null)
			System.err.println("Enumeration failed");
		else {
			String context = result.context();
			System.err.println("Context: " + context);
			while (context != null) {
				System.err.println("Pull");
				result = client.pull(options, null, URI, context);
				if (result != null)	 {
					System.out.println(result.encode("UTF-8"));
					context = result.context();
				} else
					context = null;
			}
		}
	}

}
