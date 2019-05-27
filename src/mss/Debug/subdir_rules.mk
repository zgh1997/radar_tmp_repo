################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
cli.oer4f: ../cli.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="cli.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

cycle_measure.oer4f: ../cycle_measure.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="cycle_measure.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

gtrackAlloc.oer4f: ../gtrackAlloc.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="gtrackAlloc.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

gtrackLog.oer4f: ../gtrackLog.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="gtrackLog.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

mss_main.oer4f: ../mss_main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="mss_main.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1685661375:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1685661375-inproc

build-1685661375-inproc: ../mss_mmw.cfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_50_00_10_core/xs" --xdcpath="C:/ti/bios_6_52_00_12/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.R4F -p ti.platforms.cortexR:IWR16XX:false:200 -r release -c "C:/ti/ti-cgt-arm_16.9.1.LTS" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

configPkg/linker.cmd: build-1685661375 ../mss_mmw.cfg
configPkg/compiler.opt: build-1685661375
configPkg/: build-1685661375

radarOsal_malloc.oer4f: ../radarOsal_malloc.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="radarOsal_malloc.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

task_app.oer4f: ../task_app.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="task_app.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

task_mbox.oer4f: ../task_mbox.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 -me -O3 --include_path="C:/Users/a0232274/60GH_Dev/pplcount_mss_68xx" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01" --include_path="C:/ti/mmwave_sdk_IWR6843TC_01_00_00_01/packages" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo" --include_path="C:/Users/a0232274/ti/repos/lab0015_pplcount_68xx/lab0002_pplcount_pjt/radarDemo/chains/RadarReceiverPeopleCounting/mmw_PCDemo/gtrack" --include_path="C:/ti/ti-cgt-arm_16.9.1.LTS/include" --define=_LITTLE_ENDIAN --define=SOC_XWR16XX --define=SUBSYS_MSS --define=DOWNLOAD_FROM_CCS --define=DebugP_ASSERT_ENABLED --define=MMWAVE_L3RAM_SIZE=0x40000 -g --c99 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --enum_type=packed --abi=eabi --obj_extension=.oer4f --preproc_with_compile --preproc_dependency="task_mbox.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


