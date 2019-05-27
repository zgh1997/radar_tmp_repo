-heap  0x8000000
-stack 0x2000

MEMORY
{
    L2SRAM (RWX) : org = 0x800000, len = 0x40000
    SL2_SRAM (RWX) : org = 0x5B000000, len = 0x40000
    DDR3_INPUT (RWX) : org = 0xC0000000, len = 0x1000000
    DDR3_1DWINOUT (RWX) : org = 0xC1000000, len = 0x1000000
    DDR3_1DFFTOUT (RWX) : org = 0xC2000000, len = 0x1000000
    DDR3_2DWINOUT (RWX) : org = 0xC3000000, len = 0x1000000
    DDR3_2DFFTOUT (RWX) : org = 0xC4000000, len = 0x1000000
    DDR3_POWOUT (RWX) : org = 0xC5000000, len = 0x1000000
    DDR3 (RWX) : org = 0xC8000000, len = 0x18000000
}

SECTIONS
{
	.L2heap: load >> L2SRAM
	.L2ScratchSect : load >> L2SRAM
	.L1ScratchSect : load >> L2SRAM
	.ddrHeap: load >> DDR3
	.ddrScratchSect: load >> DDR3
    .text: load >> L2SRAM
    
    GROUP (NEAR_DP)
    {
    .neardata
    .rodata 
    .bss
    } load > L2SRAM

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
