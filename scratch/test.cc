using namespace ns3;

#include <ns3/wifi-module.h>
#include <ns3/csma-module.h>
#include <ns3/core-module.h>
int
main (int argc, char *argv[])
{
	
	Ptr<WifiNetDevice> dev1 = CreateObject<WifiNetDevice> ();
	cout << "dev1: " << dev1->GetTypeId().GetGroupName() <<endl;
	Ptr<CsmaNetDevice> dev2 = CreateObject<CsmaNetDevice> ();
	cout << "dev2: " << dev2->GetTypeId().GetGroupName() <<endl;
	NetDeviceContainer con;
	con.Add (dev1);
	con.Add (dev2);
	cout << "in container:" <<endl;
	cout << "dev1: " << con.Get(0)->GetTypeId().GetGroupName() <<endl;
	cout << "dev2: " << con.Get(1)->GetTypeId().GetGroupName() <<endl;
	
	return 0;
}
