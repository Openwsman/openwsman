import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.XmlDoc;
import org.openwsman.jwsmanConstants;

public class cim_enumerate_all {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Client client = new Client(
				"http://wsman:secret@linux-3qev.localnet:5985/wsman");
		ClientOptions options = new ClientOptions();
		// op.set_dump_request();
		client.transport().set_auth_method("basic");

		System.err.println("Enumerate");
		XmlDoc result = client.enumerate(options, null, jwsmanConstants.CIM_ALL_AVAILABLE_CLASSES);
		if ((result == null) || (result.is_fault() != 0))
			System.err.println("Enumeration failed");
		else {
			String context = result.context();
			System.err.println("Context: " + context);
			while (context != null) {
				System.err.println("Pull");
				result = client.pull(options, null, jwsmanConstants.CIM_ALL_AVAILABLE_CLASSES, context);
				if (result != null)	 {
					System.out.println(result.encode("UTF-8"));
					context = result.context();
				} else
					context = null;
			}
		}
	}

}
