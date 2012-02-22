/*
* NetServiceMonitor.cpp
*
* Copyright (c) 2012 jxfengzi@gmail.com 
*
*
*
*
*/

#include "NetServiceMonitor.h"
#include "NetService.h"
#include "dns_sd.h"

#include "NetServiceThread.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#else
#include <arpa/inet.h>
#endif

using namespace ZeroConf;

//-----------------------------------------------------------------------------
// Static API
//-----------------------------------------------------------------------------

static 
void DNSSD_API address_reply (DNSServiceRef sdRef,
                              DNSServiceFlags flags, 
                              uint32_t interfaceIndex, 
                              DNSServiceErrorType errorCode, 
                              const char *hostname, 
                              const struct sockaddr *address, 
                              uint32_t ttl, 
                              void *context)
{
  NetServiceMonitor *self = (NetServiceMonitor *)context;
  self->updateIpAddressWithHostName(hostname, address, ttl);
}

static
void DNSSD_API resolve_reply (DNSServiceRef sdRef,
                              DNSServiceFlags flags, 
                              uint32_t interfaceIndex, 
                              DNSServiceErrorType errorCode,
                              const char *fullresolvename, 
                              const char *hosttarget, 
                              uint16_t opaqueport,            ///< port 
                              uint16_t txtLen,
                              const unsigned char *txtRecord, 
                              void *context )
{
  NetServiceMonitor *self = (NetServiceMonitor *)context;

  NetService *pNetService = self->getServiceWithFullResolvName(fullresolvename);
  if (pNetService)
  {
    pNetService->setHostName(hosttarget);
    pNetService->setPort(ntohs(opaqueport));
    pNetService->setTXTRecordData(txtLen, txtRecord);

    if (self->delegate())
      self->delegate()->didResolvService(pNetService);
  }

  self->m_resolvAddrRef = self->m_shareRef;

  DNSServiceGetAddrInfo (&self->m_resolvAddrRef,
                         kDNSServiceFlagsShareConnection,
                         interfaceIndex,
                         kDNSServiceProtocol_IPv4,
                         hosttarget,
                         address_reply,
                         context);
}

static 
void DNSSD_API browse_reply (DNSServiceRef sdref, 
                             const DNSServiceFlags flags, 
                             uint32_t ifIndex, 
                             DNSServiceErrorType errorCode,
                             const char *replyName, 
                             const char *replyType, 
                             const char *replyDomain, 
                             void *context)
{
  NetServiceMonitor *self = (NetServiceMonitor *)context;

  if (flags & kDNSServiceFlagsAdd) 
  {
    self->addService(replyName, replyType, replyDomain);

    self->m_resolveRef = self->m_shareRef;

    DNSServiceResolve (&(self->m_resolveRef),
      kDNSServiceFlagsShareConnection,
      ifIndex,
      replyName,
      replyType,
      replyDomain,
      resolve_reply,
      context);
  }
  else 
  {
    self->removeService(replyName, replyType, replyDomain);
  }
}

static
void DNSSD_API register_reply(DNSServiceRef       sdRef,
                              DNSServiceFlags     flags, 
                              DNSServiceErrorType errorCode, 
                              const char          *name, 
                              const char          *regtype, 
                              const char          *domain, 
                              void                *context )
{
  NetServiceMonitor *self = (NetServiceMonitor *)context;
    
  switch (errorCode)
  {
    case kDNSServiceErr_NoError:      
      if (self->delegate())
        self->delegate()->didPublish(self->getPublishService());
      break;

    case kDNSServiceErr_NameConflict: 
      if(self->delegate())
        self->delegate()->didNotPublish(self->getPublishService());
      break;

    default:                          
      if(self->delegate())
        self->delegate()->didNotPublish(self->getPublishService());
      break;
  }
}  

//-----------------------------------------------------------------------------
// Public API
//-----------------------------------------------------------------------------

NetServiceMonitor::NetServiceMonitor() 
                : m_delegate(NULL)
                , m_shareRef(NULL)
                , m_browseRef(NULL)
                , m_resolveRef(NULL)
                , m_resolvAddrRef(NULL)
                , m_registerRef(NULL)
                , m_thread(NULL)
                , m_publishService(NULL)
{
}

NetServiceMonitor::~NetServiceMonitor() 
{
  stop();
}

bool NetServiceMonitor::startMonitorForServiceOfType(const std::string &serviceType, 
                                                     const std::string &domainName)
{
  stop();

  DNSServiceErrorType err = this->startServiceBrowse(serviceType.c_str(), domainName.c_str(), this);
  if (err != kDNSServiceErr_NoError)
  {
    if (this->m_delegate)
      this->m_delegate->didNotStartMonitor(this);

    if (m_shareRef)
    {
      DNSServiceRefDeallocate(m_shareRef);
		  m_shareRef = NULL;
    }

    return false;
  }

  if (this->m_delegate)
    this->m_delegate->willStartMonitor(this);

  m_thread = new NetServiceThread(&m_shareRef, 1.0);
  m_thread->startThread();

  return true;
}

bool NetServiceMonitor::publish(NetService *service)
{
  m_publishService = service;

  if (service->getPort() == 0)
  {
    if (this->m_delegate)
      this->m_delegate->didNotPublish(service);

    return false;
  } 

  DNSServiceFlags flags	= kDNSServiceFlagsShareConnection;
  uint32_t interfaceIndex = kDNSServiceInterfaceIndexAny;		// all interfaces 

  const char *name = service->getName().c_str();
  const char *type = service->getType().c_str();
  const char *domain = service->getDomain().c_str();
  const char *hostName = service->getHostName().c_str();
  uint16_t registerPort = htons(service->getPort());

  // TXTRecord
  TXTRecordRef txtRecord;
  TXTRecordCreate(&txtRecord, 0, NULL);

  std::string deviceId    = service->getTXTRecordValueWithKey("deviceId");
  std::string deviceType  = service->getTXTRecordValueWithKey("deviceType");

	DNSServiceErrorType err = kDNSServiceErr_NoError;
  err = TXTRecordSetValue(&txtRecord, "deviceId", deviceId.size(), deviceId.c_str());
  err = TXTRecordSetValue(&txtRecord, "deviceType", deviceType.size(), deviceType.c_str());

  m_registerRef = m_shareRef;
  
  DNSServiceErrorType result = DNSServiceRegister(&m_registerRef,
                                                  flags, 
                                                  interfaceIndex, 
                                                  name,
                                                  type,
                                                  domain,
                                                  hostName, 
                                                  registerPort,
                                                  TXTRecordGetLength (&txtRecord),
                                                  TXTRecordGetBytesPtr (&txtRecord),
                                                  (DNSServiceRegisterReply)&register_reply, 
                                                  this);
  
  TXTRecordDeallocate(&txtRecord);

  if (result != kDNSServiceErr_NoError)
  {
    if (this->m_delegate)
      this->m_delegate->didNotPublish(service);

    if (m_registerRef)
    {
      DNSServiceRefDeallocate(m_registerRef);
      m_registerRef = NULL;			
    }

    return false;
  }

  if (this->m_delegate)
      this->m_delegate->willPublish(service);

  return true;
}

void NetServiceMonitor::stop()
{
  if (m_thread)
  {
    m_thread->setThreadShouldExit();
    m_thread->waitForThreadToExit(100);
    delete m_thread;
    m_thread = NULL;
  }
	
	if (m_shareRef)
  {
		DNSServiceRefDeallocate(m_shareRef);
	  m_shareRef = NULL;
  }

	if (m_browseRef)
  {
		DNSServiceRefDeallocate(m_browseRef);
	  m_browseRef = NULL;
  }
  	
	if (m_resolveRef)
  {
		DNSServiceRefDeallocate(m_resolveRef);
	  m_resolveRef = NULL;
  }
  	
	if (m_resolvAddrRef)
  {
		DNSServiceRefDeallocate(m_resolvAddrRef);
	  m_resolvAddrRef = NULL;
  }

  if (m_registerRef)
  {
		DNSServiceRefDeallocate(m_registerRef);
	  m_registerRef = NULL;
  }

  this->removeAllService();
}

//-----------------------------------------------------------------------------
// Private API
//-----------------------------------------------------------------------------

DNSServiceErrorType NetServiceMonitor::startServiceBrowse(const char *serviceType,
                                                          const char *domain,
                                                          void *context)
{
  uint32_t interfaceIndex = kDNSServiceInterfaceIndexAny;
  DNSServiceErrorType err;

  DNSServiceFlags flag = kDNSServiceFlagsShareConnection;

  err = DNSServiceCreateConnection (&m_shareRef);
  if (err != kDNSServiceErr_NoError)
    return err;

  m_browseRef = m_shareRef;

  return DNSServiceBrowse (&m_browseRef,
                           flag,
                           interfaceIndex,
                           serviceType,
                           domain,
                           browse_reply,
                           context);
}

void NetServiceMonitor::HandleEvents (DNSServiceRef *serviceRef)
{
  int dns_sd_fd  = *serviceRef ? DNSServiceRefSockFD(*serviceRef   ) : -1;
  int nfds = dns_sd_fd + 1;
  fd_set readfds;
  struct timeval tv;
  int result;

  // 1. Set up the fd_set as usual here.
  // This example thiz->dnsSdRef has no file descriptors of its own,
  // but a real application would call FD_SET to add them to the set here
  FD_ZERO(&readfds);

  // 2. Add the fd for our thiz->dnsSdRef(s) to the fd_set
  FD_SET(dns_sd_fd , &readfds);

  // 3. Set up the timeout.
  //tv.tv_sec  = LONG_TIME;
  tv.tv_sec  = 3;
  tv.tv_usec = 0;

  result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
  if (result > 0) 
  {
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    if ((*serviceRef) && FD_ISSET(dns_sd_fd , &readfds)) 
      err = DNSServiceProcessResult (*serviceRef);
  }
}

void NetServiceMonitor::addService(const char *name, const char *type, const char *domain)
{
  // Service already exist;
  NetServiceList::iterator iter;
  for (iter = m_services.begin(); iter != m_services.end(); ++iter)
  {
    NetService *pNetService = *iter;
    if (pNetService->getName() == name && pNetService->getType() == type && pNetService->getDomain() == domain)
      return;
  }
  
  // add new service
  NetService *pNetService = new NetService(name, type, domain);
      
  if (this->m_delegate)
    m_delegate->didFindService(pNetService);

  ScopedLock lock(m_servicesCriticalSection);
  m_services.push_back(pNetService);
}

void NetServiceMonitor::removeService(const char *name, const char *type, const char *domain)
{
  // Service already exist;
  NetServiceList::iterator iter;
  for (iter = m_services.begin(); iter != m_services.end(); ++iter)
  {
    NetService *pNetService = *iter;
    if (pNetService->getName() == name && pNetService->getType() == type && pNetService->getDomain() == domain)
    {
      ScopedLock lock(m_servicesCriticalSection);
      m_services.erase(iter);

      if (this->m_delegate)
        m_delegate->didRemoveService(pNetService);

      delete(pNetService);

      break;
    }
  }
}

void NetServiceMonitor::removeAllService(void)
{
  NetServiceList::iterator iter;
  for (iter = m_services.begin(); iter != m_services.end(); ++iter)
  {
    NetService *pNetService = *iter;

    ScopedLock lock(m_servicesCriticalSection);
    m_services.erase(iter);

    if (this->m_delegate)
      m_delegate->didRemoveService(pNetService);

    delete pNetService;
  }
}

NetService * NetServiceMonitor::getServiceWithFullResolvName(const char *fullResolveName)
{
  // Service already exist;
  NetServiceList::iterator iter;
  for (iter = m_services.begin(); iter != m_services.end(); ++iter)
  {
    NetService *pNetService = *iter;
    if (pNetService->getFullResolveName() == fullResolveName)
    {
      return pNetService;
    }
  }

  return NULL;
}

void NetServiceMonitor::updateIpAddressWithHostName(const char *hostName, 
                                                    const struct sockaddr *address, 
                                                    uint32_t ttl)
{
  // Service already exist;
  NetServiceList::iterator iter;
  for (iter = m_services.begin(); iter != m_services.end(); ++iter)
  {
    NetService *pNetService = *iter;
    if (pNetService->getHostName() == hostName)
    {
      pNetService->setIPAddress(address, ttl);

      if (this->m_delegate)
        m_delegate->didGetIPAddressForService(pNetService);
    }
  }
}
