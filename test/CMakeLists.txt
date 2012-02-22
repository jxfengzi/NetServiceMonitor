ADD_EXECUTABLE(test testNetServiceMonitor.cpp)
TARGET_LINK_LIBRARIES(test NetServiceMonitor_Static ${OS_LIB} ${DNSSD_LIB})
