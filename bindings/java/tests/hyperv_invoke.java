/*
 * This is a non-working example
 * 
 * It's purpose is to call invoke() with an EndpointReference parameter
 * and visualize the respective XML request for debugging purposes
 *
 * Licensed under the Openwsman license.
 */

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

public class hyperv_invoke {

  /**
   * @param args
   */
  public static void main(String[] args) {
    Client client = new Client("http://wsman:secret@localhost:5985/wsman");
    client.transport().set_auth_method(OpenWSManConstants.BASIC_AUTH_STR);

    String uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/";

    ClientOptions options = new ClientOptions();
    options.set_dump_request();
    options.add_selector("Name", "vmms");
    options.add_selector("SystemCreationClassName", "Msvm_ComputerSystem");
    options.add_selector("CreationClassName", "Msvm_VirtualSystemManagementService");
    options.add_selector("SystemName", "hyperv.host.com");

    EndPointReference epr4 = new EndPointReference(uri + "Msvm_PlannedComputerSystem", OpenWSManConstants.WSA_TO_ANONYMOUS);
    epr4.add_selector("Name", "0ED00E8D-56B8-4255-9DCD-5420D16439E4");
    epr4.add_selector("creationclassname", "Msvm_PlannedComputerSystem");

    options.add_property( "PlannedSystem", epr4 );

    XmlDoc result1 = client.get_from_epr( options, epr4 );

    XmlDoc result2 = client.invoke(options, uri + "Msvm_VirtualSystemManagementService", "RealizePlannedSystem", result1);
  }

}
