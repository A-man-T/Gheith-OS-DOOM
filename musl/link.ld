ENTRY(_start)

SECTIONS
{
   . = 0x80000000;
   .init : { *(.init) }
   .text : { *(.text) }
   .data : { *(.data) }
   .bss : { *(.bss) }
}
