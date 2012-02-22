
#include <iostream>

#include "NetServiceMonitor.h"
#include "NetService.h"

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace ZeroConf;

class MyNetServiceMonitor : public NetServiceMonitor,
                            public NetServiceMonitorDelegate
{

public:
  MyNetServiceMonitor()
  {
    setDelegate(this);
  }

private:
  virtual void didNotStartMonitor(NetServiceMonitor *monitor)
  {
    std::cout << "didNotStartMonitor: " << std::endl;
  }

  virtual void willStartMonitor(NetServiceMonitor *monitor)
  {
    std::cout << "willStartMonitor: " << std::endl;
  }

  virtual void didFindService(NetService *service)
  {
    std::cout << "didFindService: " << service->getName() << " type: " << service->getType() << " domain: " << service->getDomain() << std::endl;
  }

  virtual void didRemoveService(NetService *service)
  {
    std::cout << "didRemoveService: " << service->getName() << std::endl;
  }

  virtual void didResolvService(NetService *service)
  {
    std::cout << "didResolvService: " << service->getFullResolveName() << " hoseName: " << service->getHostName() << " port:" << service->getPort() << std::endl;
    std::cout << "deviceId: " << service->getTXTRecordValueWithKey("deviceId") << std::endl;
    std::cout << "deviceType: " << service->getTXTRecordValueWithKey("deviceType") << std::endl;
  }

  virtual void didGetIPAddressForService(NetService *service)
  {
    std::cout << "didGetIPAddressForService: " << service->getName() << " ip: " << service->getIPWithString() << std::endl;
  }

  virtual void didNotPublish(NetService *service)
  {
    std::cout << "didNotPublish: " << std::endl;
  }

  virtual void didPublish(NetService *service)
  {
    std::cout << "didPublish: " << std::endl;
  }

  virtual void willPublish(NetService *service)
  {
    std::cout << "willPublish: " << std::endl;
  }
};


int main()
{
    MyNetServiceMonitor monitor;

    monitor.startMonitorForServiceOfType("_testtxt._tcp", "local.");

    NetService service("OuyangWindows", "_testtxt._tcp", "local.");
    service.setPort(12345);
    service.setTXTRecordValueWithKey("deviceId", "000111333");
    service.setTXTRecordValueWithKey("deviceType", "DesktopPC");

    monitor.publish(&service);

    while (1)
    {
      std::cout << "hi..." << std::endl;

    #ifdef _WIN32
      Sleep(1000);
    #else
      sleep(1);
    #endif

    }

    return 0;
}
