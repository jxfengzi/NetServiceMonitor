# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

#
# dns-sd -- by ouyang, 2010-2-25
#
include $(CLEAR_VARS)

LOCAL_MODULE := myMonitor
LOCAL_SRC_FILES := ../../src/CriticalSection.cpp        \
                   ../../src/NetService.cpp 			\
                   ../../src/NetServiceMonitor.cpp 			\
                   ../../src/Thread.cpp 					\
                   ../../src/NetServiceThread.cpp

LOCAL_CFLAGS := -I../../3rdpart/include		\
                -L../../3rdpart/lib/linux32 \
		        -Wall   					\
		        -DHAVE_LINUX 				\
		        -Os 			    		\
		        -DMDNS_DEBUGMSGS=0  		\
                -DANDROID 

LOCAL_SHARED_LIBRARIES := dns_sd 

include $(BUILD_EXECUTABLE)
