import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.XmlDoc;
import org.openwsman.OpenWSManConstants;

public class cim_linux_operatingsystem_exec_cmd {
	public static final String URI = OpenWSManConstants.XML_NS_CIM_CLASS + "/Linux_OperatingSystem";

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Client client = new Client("http://wsman:secret@localhost:5985/wsman");
		ClientOptions options = new ClientOptions();
		options.set_dump_request();
		client.transport().set_auth_method(OpenWSManConstants.BASIC_AUTH_STR);

		XmlDoc result = client.enumerate(options, null, URI);
		if ((result == null) || result.isFault()) {
			System.err.println("Enumeration failed: "
					+ ((result != null) ? result.fault().reason() : "?"));
			System.exit(1);
		}

		String context = result.context();
		System.out.println("Context: " + context);
		result = client.pull(options, null, URI, context);
		if (result == null || result.isFault()) {
			System.err.println("Pull failed: "
					+ ((result != null) ? result.fault().reason() : "?"));
			System.exit(1);
		}

		System.err.println(result.encode("UTF-8"));

		String CSCreationClassname = result.root().find(URI,
				"CSCreationClassName", 1).toString();
		String CSName = result.root().find(URI, "CSName", 1).toString();
		String CreationClassname = result.root().find(URI, "CreationClassName",
				1).toString();
		String Name = result.root().find(URI, "Name", 1).toString();

		options.add_selector("CSCreationClassname", CSCreationClassname);
		options.add_selector("CSName", CSName);
		options.add_selector("CreationClassname", CreationClassname);
		options.add_selector("Name", Name);
		options.add_property("cmd", "ls /");

		result = client.invoke(options, URI, "execCmd", null);
		if (result == null || result.isFault()) {
			System.err.println("Invoke failed: "
					+ ((result != null) ? result.fault().reason() : "?"));
			System.exit(1);
		}

		System.out.println(result.encode("UTF-8"));
	}
}
