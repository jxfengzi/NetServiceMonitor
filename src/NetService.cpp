/*
* NetService.cpp
*
* Copyright (c) 2012 jxfengzi@gmail.com 
*
*
*
*
*/

#include "NetService.h"

#include <iostream>

#ifdef _WIN32
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif

using namespace ZeroConf;

NetService::NetService(const std::string &name, const std::string &type, const std::string &domain)
           : m_domain(domain)
           , m_type(type)
           , m_name(name)
           , m_port(0)
           , m_ip(0)
{
  m_fullResolveName = name;
  m_fullResolveName.append(".");
  m_fullResolveName.append(type);
  m_fullResolveName.append(domain);
}

NetService::~NetService()
{
}

void NetService::setIPAddress(const struct sockaddr *address, uint32_t ttl)
{
  const char * ipChar = (const char *)inet_ntoa (((struct sockaddr_in *)address)->sin_addr);
  unsigned long ip = inet_addr (ipChar);

  m_ipString = ipChar;
  m_ip       = ip;
}

void NetService::setTXTRecordData(uint16_t txtLen, const unsigned char *txtRecord)
{
  m_TXTRecords.clear();
  
  if (txtLen < 2)
    return;

  int i = 0;
  char buf[256];
  char key[256];
  char value[256];
  
  unsigned char length = txtRecord[i];
  while (i < txtLen)
  {
    const unsigned char *start = txtRecord + (i+1);

    memset(buf, 0, 256);
    memcpy(buf, start, length);

    i = i + length + 1;
    length = txtRecord[i];

    char *p = strstr(buf, "=");
    if (p)
    {
      memset(key, 0, 256);
      memcpy(key,  buf, p - buf);

      p++;
      memset(value, 0, 256);
      memcpy(value,  p, strlen(p));

      m_TXTRecords.insert(std::pair<std::string, std::string>(key, value));
    }
  }
}

std::string & NetService::getTXTRecordValueWithKey(const char *key)
{
  static std::string value;

  TXTRecordMap::iterator pos = m_TXTRecords.find(key);
  if (pos != m_TXTRecords.end())
    value = pos->second;

  return value;
}

void NetService::setTXTRecordValueWithKey(const char *key, const char *value)
{
  TXTRecordMap::iterator pos = m_TXTRecords.find(key);
  if (pos == m_TXTRecords.end())
  {
    m_TXTRecords.insert(std::pair<std::string, std::string>(key, value));
  }
}