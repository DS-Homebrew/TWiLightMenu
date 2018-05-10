OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
	ewram	: ORIGIN = 0x02000000, LENGTH = 16M
}


PHDRS {
	main    PT_LOAD FLAGS(7);
}

SECTIONS
{

	.crt0	:
	{
		__text_start = . ;
		KEEP (*(.crt0))
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0x00

	.plt : { *(.plt) } >ewram :main = 0xff

	.init :
	{
		KEEP (*(SORT_NONE(.init)))
	} >ewram :main

	.text :   /* ALIGN (4): */
	{
		*(EXCLUDE_FILE(*.itcm* *.vectors* *.twl*) .text)
		*(EXCLUDE_FILE(*.itcm* *.vectors* *.twl*) .stub)
		*(EXCLUDE_FILE(*.itcm* *.vectors* *.twl*) .text.*)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(EXCLUDE_FILE(*.twl*) .gnu.warning)
		*(EXCLUDE_FILE(*.twl*) .gnu.linkonce.t*)
		*(.glue_7)
		*(.glue_7t)
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff

	.fini           :
	{
		KEEP (*(.fini))
	} >ewram :main =0xff

	__text_end = . ;

	.rodata :
	{
		*(EXCLUDE_FILE(*.twl*) .rodata)
		*all.rodata*(*)
		*(EXCLUDE_FILE(*.twl*) .roda)
		*(EXCLUDE_FILE(*.twl*) .rodata.*)
		*(EXCLUDE_FILE(*.twl*) .gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff

	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ewram :main
 	__exidx_start = .;
	ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >ewram :main
 	__exidx_end = .;

	/*	Ensure the __preinit_array_start label is properly aligned.  We
		could instead move the label definition inside the section, but
		the linker would then create the section even if it turns out to
		be empty, which isn't pretty.  */

	. = ALIGN(32 / 8);

	PROVIDE (__preinit_array_start = .);
	.preinit_array     : { KEEP (*(.preinit_array)) } >ewram :main = 0xff
	PROVIDE (__preinit_array_end = .);
	PROVIDE (__init_array_start = .);
	.init_array     :
	{
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
	} >ewram :main = 0xff
	PROVIDE (__init_array_end = .);
	PROVIDE (__fini_array_start = .);
	.fini_array     :
	{
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
	} >ewram :main = 0xff

	PROVIDE (__fini_array_end = .);

	.ctors :
	{
	/*	gcc uses crtbegin.o to find the start of the constructors, so
		we make sure it is first.  Because this is a wildcard, it
		doesn't matter if the user does not actually link against
		crtbegin.o; the linker won't look for a file to match a
		wildcard.  The wildcard also means that it doesn't matter which
		directory crtbegin.o is in.  */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff

	.eh_frame :
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff

	.gcc_except_table :
	{
		*(.gcc_except_table)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff
	.jcr            : { KEEP (*(.jcr)) } >ewram :main = 0
	.got            : { *(.got.plt) *(.got) *(.rel.got) } >ewram :main = 0

	.ewram ALIGN(4) : 
	{
		__ewram_start = ABSOLUTE(.) ;
		*(.ewram)
		*ewram.*(.text)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram :main = 0xff


	.data ALIGN(4) :
	{
		__data_start = ABSOLUTE(.);
		*(EXCLUDE_FILE(*.twl*) .data)
		*(EXCLUDE_FILE(*.twl*) .data.*)
		*(EXCLUDE_FILE(*.twl*) .gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
		__data_end = ABSOLUTE(.) ;
	} >ewram :main = 0xff

	__bss_vma = . ;

	.bss __bss_vma (NOLOAD): 
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(EXCLUDE_FILE(*.twl*) .dynbss)
		*(EXCLUDE_FILE(*.twl*) .gnu.linkonce.b*)
		*(EXCLUDE_FILE(*.twl*) .bss*)
		*(EXCLUDE_FILE(*.twl*) COMMON)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
		__bss_end__ = ABSOLUTE(.) ;
		__end__ = ABSOLUTE(.) ;
	} >ewram :NONE


	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to the beginning
		of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.stack 0x80000 : { _stack = .; *(.stack) }
	/* These must appear regardless of  .  */
}
