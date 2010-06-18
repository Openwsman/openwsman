import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.Filter;
import org.openwsman.XmlDoc;
import org.openwsman.jwsmanConstants;

public class cim_enumerate_operating_system {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Client client = new Client(
				"http://wsman:secret@linux-3qev.localnet:5985/wsman");
		ClientOptions options = new ClientOptions();
		options.set_dump_request();
		client.transport().set_auth_method(jwsmanConstants.BASIC_AUTH_STR);

		Filter filter = new Filter();
		filter.wql("SELECT * FROM CIM_OperatingSystem");
		
		XmlDoc result = client.enumerate(options, filter, jwsmanConstants.CIM_ALL_AVAILABLE_CLASSES);
		if ((result == null) || (result.is_fault() != 0))
			System.err.println("Enumeration failed: "
					+ ((result != null) ? result.fault().detail() : "?"));
		else {
			String context = result.context();
			while (context != null) {
				System.out.println("Context: " + context);
				result = client.pull(options, null, jwsmanConstants.CIM_ALL_AVAILABLE_CLASSES, context);
				if (result == null || (result.is_fault() != 0))	 {
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
