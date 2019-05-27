###################################################################################
# clustering Unit Test on R4 Makefile
###################################################################################
.PHONY: mssTest mssTestClean

###################################################################################
# Setup the VPATH:
###################################################################################
vpath %.c $(MMWAVE_DEMO_DEV_PATH)/radarDemo/modules/clustering/dbscan/src
vpath %.c $(MMWAVE_DEMO_DEV_PATH)/radarDemo/modules/clustering/dbscan/src/test/cortexR4/ar16xx
vpath %.c $(MMWAVE_DEMO_DEV_PATH)/radarDemo/modules/utilities

###################################################################################
# The clustering Unit Test uses the common libraries + Test Logger Library
###################################################################################
clustering_UNIT_TEST_STD_LIBS = $(R4F_COMMON_STD_LIB)									\
						 -llibtestlogger_$(MMWAVE_SDK_DEVICE).$(R4F_LIB_EXT)
clustering_UNIT_TEST_LOC_LIBS = $(R4F_COMMON_LOC_LIB)									\
						 -i$(MMWAVE_SDK_INSTALL_PATH)/ti/utils/testlogger/lib

###################################################################################
# Unit Test Files
###################################################################################
clustering_UNIT_TEST_CFG       = mss.cfg
clustering_UNIT_TEST_CMD       = $(MMWAVE_SDK_INSTALL_PATH)/ti/platform/ar16xx
clustering_UNIT_TEST_CONFIGPKG = mss_configPkg_$(MMWAVE_SDK_DEVICE)
clustering_UNIT_TEST_MAP       = $(MMWAVE_SDK_DEVICE)_clustering_mss.map
clustering_UNIT_TEST_OUT       = $(MMWAVE_SDK_DEVICE)_clustering_mss.$(R4F_EXE_EXT)
clustering_UNIT_TEST_BIN       = $(MMWAVE_SDK_DEVICE)_clustering_mss.bin
clustering_UNIT_TEST_APP_CMD   = mss_clustering_linker.cmd
clustering_UNIT_TEST_SOURCES   = 	main.c \
								radarOsal_malloc.c \
								RADARDEMO_clusteringDBscan.c \
								RADARDEMO_clusteringDBscan_priv.c
                               
clustering_UNIT_TEST_DEPENDS   = $(addprefix $(PLATFORM_OBJDIR)/, $(clustering_UNIT_TEST_SOURCES:.c=.$(R4F_DEP_EXT)))
clustering_UNIT_TEST_OBJECTS   = $(addprefix $(PLATFORM_OBJDIR)/, $(clustering_UNIT_TEST_SOURCES:.c=.$(R4F_OBJ_EXT)))

###################################################################################
# RTSC Configuration:
###################################################################################
r4.cfg: $(R4_CFG)
	@echo 'Configuring RTSC packages...'
	$(XS) --xdcpath="$(XDCPATH)" xdc.tools.configuro $(R4F_XSFLAGS) -o $(clustering_UNIT_TEST_CONFIGPKG) $(clustering_UNIT_TEST_CFG)
	@echo 'Finished configuring packages'
	@echo ' '

###################################################################################
# Build Unit Test:
###################################################################################
mssTest: BUILD_CONFIGPKG=$(clustering_UNIT_TEST_CONFIGPKG)
#mssTest: R4F_CFLAGS += --cmd_file=$(BUILD_CONFIGPKG)/compiler.opt --c99 --gen_profile_info -i$(MMWAVE_DEMO_DEV_PATH)/radarDemo -i$(MMWAVE_DEMO_DEV_PATH)/api/notarget
mssTest: R4F_CFLAGS += --cmd_file=$(BUILD_CONFIGPKG)/compiler.opt --c99 -d_LITTLE_ENDIAN -i$(MMWAVE_DEMO_DEV_PATH)/radarDemo -i$(MMWAVE_DEMO_DEV_PATH)/api/notarget
mssTest: buildDirectories r4.cfg $(clustering_UNIT_TEST_OBJECTS)
	$(R4F_LD) $(R4F_LDFLAGS) $(clustering_UNIT_TEST_LOC_LIBS) $(clustering_UNIT_TEST_STD_LIBS) 	\
	-l$(clustering_UNIT_TEST_CONFIGPKG)/linker.cmd --map_file=$(clustering_UNIT_TEST_MAP) 		\
	$(clustering_UNIT_TEST_OBJECTS) $(PLATFORM_R4F_LINK_CMD) $(clustering_UNIT_TEST_APP_CMD) 		\
	$(R4F_LD_RTS_FLAGS) -o $(clustering_UNIT_TEST_OUT)
	@echo '******************************************************************************'
	@echo 'Built the clustering R4 Unit Test OUT & BIN Formats'
	@echo '******************************************************************************'

###################################################################################
# Cleanup Unit Test:
###################################################################################
mssTestClean:
	@echo 'Cleaning the clustering R4 Unit Test objects'
	@$(DEL) $(clustering_UNIT_TEST_OBJECTS) $(clustering_UNIT_TEST_OUT) $(clustering_UNIT_TEST_BIN)
	@$(DEL) $(clustering_UNIT_TEST_MAP) $(R4_DEPENDS)
	@echo 'Cleaning the clustering R4 Unit RTSC package'
	@$(DEL) $(clustering_UNIT_TEST_CONFIGPKG)
	@$(DEL) $(PLATFORM_OBJDIR)

###################################################################################
# Dependency handling
###################################################################################
-include $(clustering_UNIT_TEST_DEPENDS)

