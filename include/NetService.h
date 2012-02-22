/*
* NetService.h
*
* Copyright (c) 2012 jxfengzi@gmail.com 
*
*
*
*
*/

#ifndef __NetService_H__
#define __NetService_H__

#include <string>
#include <map>
#include "dns_sd.h"

namespace ZeroConf 
{

  class NetService
  {
  public:
    NetService(const std::string &name, const std::string &type, const std::string &domain);
    ~NetService();

    std::string & getName(void)             { return m_name; }
    std::string & getType(void)             { return m_type; }
    std::string & getDomain(void)           { return m_domain; }

    std::string & getHostName(void)         { return m_hostName; }
    uint16_t      getPort(void)             { return m_port; }
    unsigned long getIp(void)               { return m_ip; }
    std::string & getIPWithString(void)     { return m_ipString; }
    std::string & getFullResolveName(void)  { return m_fullResolveName; }

    void setName(const char *name)          { m_name = name; }
    void setType(const char *type)          { m_type = type; }
    void setDomain(const char *domain)      { m_domain = domain; }

    void setHostName(const char *hostName)  { m_hostName = hostName; }
    void setPort(uint16_t port)             { m_port = port; }

    void setIPAddress(const struct sockaddr *address, uint32_t ttl);

    void setTXTRecordData(uint16_t txtLen, const unsigned char *txtRecord);
    std::string & getTXTRecordValueWithKey(const char *key);

    void setTXTRecordValueWithKey(const char *key, const char *value);

  public:
    std::string     m_domain;
    std::string     m_type;
    std::string     m_name;
    std::string     m_fullResolveName;

    std::string     m_hostName;
    uint16_t        m_port;

    unsigned long   m_ip;
    std::string     m_ipString;

    typedef std::map<std::string, std::string> TXTRecordMap;
    TXTRecordMap    m_TXTRecords;
  };

}






#endif // __NetService_H__
