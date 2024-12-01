OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x7C00;
    .text : { *(.text*) }
    /DISCARD/ : { *(*) }
}

ASSERT(SIZEOF(.text) == 512, "Size of bootloader is not 512 bytes");
