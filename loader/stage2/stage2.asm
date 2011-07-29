;**************************************************************************************************
;
;		ReaverOS
;		stage2.asm
;		second stage bootloader
;
;**************************************************************************************************

bits	16			; still 16 bit mode
org		0x0500

entry:
	jmp		main

; add some nulls...
times 8 - ($-$$) db 0

;**************************************************************************************************
;
;		Kernel loader variables
;
;**************************************************************************************************
kernelsize		dw 0
selfsize		dw 0
bootdrive		dw 0

%define		addressp	0x100000		; it'll reside at 1 MB address in memory (initrd just after kernel)
%define		addressr	0x3000

msg			db 0x0a, 0x0d, "Hello, this is ReaverOS stage 2 bootloader.", 0x0a, 0x0d, "Loading kernel...", 0
initrd		db 0x0a, 0x0d, "Loading initrd...", 0
fail		db 0x0a, 0x0d, "FATAL ERROR during stage 2. Press any key to reboot...", 0x0a, 0x0d, 0
progress	db ".", 0

;**************************************************************************************************
;
;		Includes
;
;**************************************************************************************************
%include	"stdio.asm"
%include	"gdt.asm"
%include	"floppy.asm"
%include	"a20.asm"

;**************************************************************************************************
;
;		main function
;		setups gdt, a20 and loads and executes kernel
;
;**************************************************************************************************
main:
	mov		[bootdrive], ax
	mov		[selfsize], bx

	cli
	xor		ax, ax
	mov		ds, ax
	mov		es, ax
	
	mov		ax, 0x9000
	mov		ss, ax
	mov		sp, 0xffff
	sti
	
	mov		si, msg
	call	print16
	
	call	enable_a20
	call	install_gdt
	sti
	
	mov		ecx, [kernelsize]
	mov		eax, 1
	add		eax, dword [selfsize]
	
	mov		ebx, addressr
	call	read_sectors

	mov		si, initrd
	call	print16
	
	mov		eax, kernelsize
	mov		ebx, 4
	mul		ebx
	mov		ebx, addressr
	add		ebx, eax

	add		eax, dword [selfsize]
	add		eax, dword [kernelsize]
	mov		ecx, 1
	call	read_sectors
	
	inc		eax
	mov		ecx, dword [ebx]
	add		ebx, 512
	call	read_sectors

;**************************************************************************************************
;
;		Enter protected mode
;
;**************************************************************************************************
	cli
	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax
	jmp 	0x8:stage3
	
;**************************************************************************************************
;
;		Stage 3 entry point
;		Moves kernel to it's final address and executes it
;
;**************************************************************************************************
bits	32

msg32		db "Kernel and initrd loaded, initializing environment...", 0x0a, 0

stage3:
	mov		ax, 0x10
	mov		ds, ax
	mov		ss, ax
	mov		es, ax
	mov		esp, 0x90000			; here the stack begin

	call	clear_screen
	call	movcur
	
	mov		ebx, msg32
	call	print
	
	mov		eax, dword [kernelsize]
	movzx	ebx, word [sectorsize]
	mul		ebx
	mov		ebx, 4
	div		ebx
	cld
	mov		esi, addressr
	mov		edi, addressp
	mov		ecx, eax
	rep		movsd
	; kernel moved, but you need some knowledge to understand that short code :D
	
	hlt