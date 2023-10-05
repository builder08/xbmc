set(SOURCES CocoaInterface.mm
            CPUInfoOsx.cpp
            GPUInfoMacOS.cpp
            HotKeyController.m
            PlatformDarwinOSX.cpp)

set(HEADERS CocoaInterface.h
            CPUInfoOsx.h
            GPUInfoMacOS.h
            HotKeyController.h
            PlatformDarwinOSX.h)

core_add_library(platform_osx)
