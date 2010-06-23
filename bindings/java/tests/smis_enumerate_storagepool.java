import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.openwsman.Client;
import org.openwsman.ClientOptions;
import org.openwsman.EndPointReference;
import org.openwsman.Filter;
import org.openwsman.XmlDoc;
import org.openwsman.OpenWSManConstants;
import org.openwsman.XmlNode;

public class smis_enumerate_storagepool {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Client client = new Client(
				"http://wsman:secret@localhost:5985/wsman");
		client.transport().set_auth_method(OpenWSManConstants.BASIC_AUTH_STR);
		
		EndPointReference epr = cim_enumerate_computersystem(client);
		if (epr == null) {
			System.err.println("CIM_ComputerSystem not found!");
			System.exit(1);			
		}
		
		if (!check_smis_required_profiles(client)) {
			System.err.println("Array profile not registered!");
			System.exit(1);
		}
		
		Map<String, String> map = smis_enumerate_storagepools(client, epr);
		System.out.println("List of StoragePools:");
		for (Entry<String,String> e: map.entrySet())
			System.out.println(e.getKey() + ": " + e.getValue());
	}

	/*
	 * Get a reference to the instance implementing CIM_ComputerSystem and
	 * return a valid EndPointReference for use in Enumeration of the StoragePools.
	 */
	private static EndPointReference cim_enumerate_computersystem(Client client) {
		final String URI = OpenWSManConstants.XML_NS_CIM_CLASS + "/CIM_ComputerSystem";

		ClientOptions options = new ClientOptions();
//		options.set_dump_request();
		options.add_selector(OpenWSManConstants.CIM_NAMESPACE_SELECTOR,
				"root/cimv2");

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
				
//			    System.err.println(result.root().toXML());\
				
				/* We are only interested in the CIM payload */
				XmlNode body = result.root().find(OpenWSManConstants.XML_NS_ENUMERATION,
						OpenWSManConstants.WSENUM_ITEMS, 1);
				XmlNode elem = body.child();
//				System.err.println(elem.name() + ", " + elem.ns());
				
//				XmlNode name = elem.get("Name");
//				XmlNode ccname = elem.get("CreationClassName");
//				System.err.println(name + " (" + ccname + ")");

			    EndPointReference epr = new EndPointReference(elem.ns(), null);
				epr.add_selector("Name", elem.get("Name").toString());
				epr.add_selector("CreationClassName", elem.get("CreationClassName").toString());
//				System.err.println(epr.toString());

				if ("Linux_ComputerSystem".equals(elem.get("CreationClassName").toString()))
					return epr;
				
				context = result.context();
			}
		}

		return null;
	}

	/*
	 * Check if the CIM_ComputerSystem has the required SMI-S profiles registered.
	 */
	private static boolean check_smis_required_profiles(Client client)
	{
		final String URI = OpenWSManConstants.XML_NS_CIM_CLASS + "/CIM_RegisteredProfile";
		ClientOptions options = new ClientOptions();
//		options.set_dump_request();
		options.add_selector(OpenWSManConstants.CIM_NAMESPACE_SELECTOR,
				"root/interop");

		Filter filter = new Filter();
		filter.wql("SELECT * FROM OMC_RegisteredSMIProfile");

		XmlDoc result = client.enumerate(options, filter, URI);
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

				XmlNode name = result.root().find(null,"RegisteredName", 1);
				XmlNode orgs = result.root().find(null,"RegisteredOrganization", 1);
				XmlNode version = result.root().find(null,"RegisteredVersion", 1);
				
				if (name != null && "Array".equalsIgnoreCase(name.toString())
						&& orgs != null && "11".equalsIgnoreCase(orgs.toString())
						&& version != null && 	"1.2.0".equalsIgnoreCase(version.toString()))
					return true;
					
				context = result.context();
			}
		}

		return false;
	}
	
	private static Map<String, String> smis_enumerate_storagepools(Client client, EndPointReference epr) {
		final String URI = OpenWSManConstants.CIM_ALL_AVAILABLE_CLASSES;
		Map<String,String> map = new HashMap<String,String>();

		ClientOptions options = new ClientOptions();
		options.set_dump_request();
		options.add_selector(OpenWSManConstants.CIM_NAMESPACE_SELECTOR,
				"root/cimv2");

		Filter filter = new Filter();
//		filter.associators(epr, "OMC_HostedStoragePool", "OMC_StoragePool",
		filter.associators(epr, "CIM_HostedStoragePool", "CIM_StoragePool",
				"GroupComponent", "PartComponent",
				new String[] { "PoolID", "TotalManagedSpace", "RemainingManagedSpace"});

		XmlDoc result = client.enumerate(options, filter, URI);
		if (result == null || result.isFault())
			System.err.println("Enumeration failed: "
					+ ((result != null) ? result.fault().reason() : "?"));
		else {
			String context = result.context();
			while (context != null) {
				result = client.pull(options, filter, URI, context);
				if (result == null || result.isFault())	 {
					System.err.println("Pull failed: " +
							((result != null) ? result.fault().reason() : "?"));
					context = null;
					continue;
				}
				
//				System.err.println("Pull Result:\n" + result);

				XmlNode name = result.root().find(null,"PoolID", 1);
				XmlNode tsize = result.root().find(null,"TotalManagedSpace", 1);
				XmlNode size = result.root().find(null,"RemainingManagedSpace", 1);
				
//				System.err.println(
//						(name == null ? "?" : name.text()) +
//						" (" +
//						(size == null ? "?" : size.text()) +
//						")");

				if (name != null && size != null)
					map.put(name.toString(), tsize.toString() + "/" + size.toString());
				context = result.context();
			}
		}

		return map;
	}
}
