#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/bios_6_52_00_12/packages
override XDCROOT = C:/ti/xdctools_3_50_00_10_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/bios_6_52_00_12/packages;C:/ti/xdctools_3_50_00_10_core/packages;..
HOSTOS = Windows
endif
