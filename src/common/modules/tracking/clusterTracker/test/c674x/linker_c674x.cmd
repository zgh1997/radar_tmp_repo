-heap  0x20000
-stack 0x2000

MEMORY
{
    L2SRAM (RWX) : org = 0x007e0000, len = 0x40000
    L3SRAM (RWX) : org = 0x20000000, len = 0x000a0000
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

    .sysmem: load > L3SRAM
    .ddrScratchSect: load >> L3SRAM

}
