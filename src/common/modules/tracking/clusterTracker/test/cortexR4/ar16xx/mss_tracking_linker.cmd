/*----------------------------------------------------------------------------*/
/* Linker Settings                                                            */
--retain="*(.intvecs)"

/*----------------------------------------------------------------------------*/
/* Section Configuration                                                      */
SECTIONS
{
    systemHeap : {} > 0x08000000
    .L2heap   : {} > DATA_RAM ALIGN(32)
    .ppdata   : {} > DATA_RAM ALIGN(32)
    .DDRheap   : {} > L3_RAM ALIGN(32)
    .L2ScratchSect   : {} > DATA_RAM ALIGN(32)
    .ddrScratchSect   : {} > L3_RAM ALIGN(32)
}
/*----------------------------------------------------------------------------*/

