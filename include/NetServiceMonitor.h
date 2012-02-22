/*
* NetServiceMonitor.h
*
* Copyright (c) 2012 jxfengzi@gmail.com 
*
*
*
*
*/

#ifndef __NetServiceMonitor_H__
#define __NetServiceMonitor_H__

#include <string>
#include <list>
#include <dns_sd.h>
#include "CriticalSection.h"

namespace ZeroConf 
{

  class NetService;
  class NetServiceMonitor;
  class NetServiceThread;

  class NetServiceMonitorDelegate
  {
  public:
    virtual ~NetServiceMonitorDelegate() {}

    virtual void didNotStartMonitor(NetServiceMonitor *monitor) = 0;
    virtual void willStartMonitor(NetServiceMonitor *monitor) = 0;

    virtual void didFindService(NetService *service) = 0;
    virtual void didRemoveService(NetService *service) = 0;

    virtual void didResolvService(NetService *service) = 0;
    virtual void didGetIPAddressForService(NetService *service) = 0;

    virtual void didNotPublish(NetService *service) = 0;
    virtual void willPublish(NetService *service) = 0;
    virtual void didPublish(NetService *service) = 0;
  };

  class NetServiceMonitor
  {
  public: 
    NetServiceMonitor();
    virtual ~NetServiceMonitor();

    void setDelegate(NetServiceMonitorDelegate *delegate) { m_delegate = delegate; }
    NetServiceMonitorDelegate * delegate(void) { return m_delegate; }

    bool startMonitorForServiceOfType(const std::string &serviceType, 
                                      const std::string &domainName);

    // name 不能有空格
    bool publish(NetService *service);

    void stop(void);

    void addService(const char *name, const char *type, const char *domain);
    void removeService(const char *name, const char *type, const char *domain);
    void removeAllService(void);

    NetService * getServiceWithFullResolvName(const char *fullResolveName);
    void updateIpAddressWithHostName(const char *hostName, 
                                     const struct sockaddr *address, 
                                     uint32_t ttl);

    NetService * getPublishService(void) { return m_publishService; }

  public:
    DNSServiceRef   m_shareRef;
    DNSServiceRef   m_browseRef;
    DNSServiceRef   m_resolveRef;
    DNSServiceRef   m_resolvAddrRef;
    DNSServiceRef   m_registerRef;

  private:
    DNSServiceErrorType startServiceBrowse(const char *serviceType,
                                           const char *domain,
                                           void *context);

    void HandleEvents (DNSServiceRef *ref);

  private:
    NetServiceMonitorDelegate        *m_delegate;
    
    typedef std::list<NetService *>   NetServiceList;
    NetServiceList                    m_services;
    CriticalSection                   m_servicesCriticalSection;

    NetServiceThread                 *m_thread;

    NetService                       *m_publishService;
  };
}






#endif // __NetServiceMonitor_H__
