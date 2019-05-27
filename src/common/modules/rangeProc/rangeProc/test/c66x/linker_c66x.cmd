-heap  0x8000000
-stack 0x2000

MEMORY
{
    L2SRAM (RWX) : org = 0x800000, len = 0x40000
    SL2_SRAM (RWX) : org = 0x5B000000, len = 0x40000
    DDR3_INPUT (RWX) : org = 0x80000000, len = 0x1000000
    DDR3_1DWINOUT (RWX) : org = 0x81000000, len = 0x1000000
    DDR3_1DFFTOUT (RWX) : org = 0x82000000, len = 0x1000000
    DDR3_2DWINOUT (RWX) : org = 0x83000000, len = 0x1000000
    DDR3_2DFFTOUT (RWX) : org = 0x84000000, len = 0x1000000
    DDR3_POWOUT (RWX) : org = 0x85000000, len = 0x1000000
    DDR3 (RWX) : org = 0x88000000, len = 0x18000000
}

SECTIONS
{
	.L2heap: load >> L2SRAM
	.L2ScratchSect : load >> L2SRAM
	.ddrHeap: load >> DDR3
	.ddrScratchSect: load >> DDR3
    .text: load >> L2SRAM
    
    GROUP (NEAR_DP)
    {
    .neardata
    .rodata 
    .bss
    } load > L2SRAM

	.testVec_input: load >> DDR3_INPUT
	.testVec_1dwinout: load >> DDR3_1DWINOUT
	.testVec_2dwinout: load >> DDR3_2DWINOUT
	.testVec_1dfftout: load >> DDR3_1DFFTOUT
	.testVec_2dfftout: load >> DDR3_2DFFTOUT
	.testVec_powout: load >> DDR3_POWOUT
	.testVec_far: load >> DDR3
	.PDPout: load >> DDR3
	.localBuffers: load >> L2SRAM
    .far: load >> L2SRAM
    .fardata: load >> L2SRAM
    .data: load >> L2SRAM 
    .switch: load >> L2SRAM
    .stack: load > L2SRAM
    .args: load > L2SRAM align = 0x4, fill = 0 {_argsize = 0x200; }
    .cinit: load > L2SRAM
    .const: load > L2SRAM START(const_start) SIZE(const_size)
    .pinit: load > L2SRAM
    .cio: load >> L2SRAM
	.init_array: load > L2SRAM
    xdc.meta: load >> L2SRAM, type = COPY

    .sysmem: load > DDR3
	.ddrScratchSect: load >> DDR3
}
