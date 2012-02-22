#-------------------------------------------------------------------------------------
#
# module: NetServiceMonitor
#
#-------------------------------------------------------------------------------------
SET(SRC ../include/CriticalSection.h
	../include/NetService.h
	../include/NetServiceMonitor.h
	../include/Thread.h
	../include/NetServiceThread.h
        CriticalSection.cpp
        NetService.cpp
        NetServiceMonitor.cpp
        Thread.cpp
	NetServiceThread.cpp)

ADD_LIBRARY(NetServiceMonitor_Static STATIC ${SRC})
