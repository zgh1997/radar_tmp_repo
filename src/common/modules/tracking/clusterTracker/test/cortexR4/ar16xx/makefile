###################################################################################
# HWA makefile
###################################################################################
include $(MMWAVE_SDK_INSTALL_PATH)/ti/common/mmwave_sdk.mak

##################################################################################
# SOC Specific Test Targets
##################################################################################
include mssTest.mak

###################################################################################
# Standard Targets which need to be implemented by each mmWave SDK module. This
# plugs into the release scripts.
###################################################################################
.PHONY: all clean test testClean help


###################################################################################
# Test Targets:
# AR14xx: Build the MSS Unit Test
###################################################################################
testClean: 	mssTestClean
test: 		mssTest

# Clean: This cleans all the objects
clean: testClean

# Build driver and all the unit tests
all: test

# Help: This displays the MAKEFILE Usage.
help:
	@echo '****************************************************************************************'
	@echo '* Makefile Targets for the tracking '
	@echo 'clean                -> Clean out all the objects'
	@echo 'test                 -> Builds all the unit tests for the SOC'
	@echo 'testClean            -> Cleans the unit tests for the SOC'
	@echo '****************************************************************************************'

