
#include <ns3/wifi-module.h>
#include <ns3/csma-module.h>
#include <ns3/core-module.h>

using namespace ns3;

int
main (int argc, char *argv[])
{
	
	Ptr<WifiNetDevice> dev1 = CreateObject<WifiNetDevice> ();
	std::cout << "dev1: " << dev1->GetTypeId().GetGroupName() <<std::endl;
	Ptr<CsmaNetDevice> dev2 = CreateObject<CsmaNetDevice> ();
	std::cout << "dev2: " << dev2->GetTypeId().GetGroupName() <<std::endl;
	NetDeviceContainer con;
	con.Add (dev1);
	con.Add (dev2);
	std::cout << "in container:" <<std::endl;
	std::cout << "dev1: " << con.Get(0)->GetTypeId().GetGroupName() <<std::endl;
	std::cout << "dev2: " << con.Get(1)->GetTypeId().GetGroupName() <<std::endl;
	
	return 0;
}
