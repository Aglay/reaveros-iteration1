;**************************************************************************************************
;
;       ReaverOS
;       stage2.asm
;       second stage bootloader
;
;**************************************************************************************************

bits    16          ; still 16 bit mode
org     0x0500

entry:
    jmp     main

; add some nulls...
times 8 - ($-$$) db 0

;**************************************************************************************************
;
;       Kernel loader variables
;
;**************************************************************************************************
kernelsize      dw 0
initrdsize      dw 0
selfsize        dw 0
bootdrive       dw 0

remaining       dw 0
done            dw 0

%define         addressp    0x10000
; it'll reside at 1 MB address in memory
%define         addressr    0x1000

msg         db 0x0a, 0x0d, "Hello, this is ReaverOS stage 2 bootloader.", 0x0a, 0x0d, "Loading kernel and initrd...", 0
fail        db 0x0a, 0x0d, "FATAL ERROR during stage 2. Press any key to reboot...", 0x0a, 0x0d, 0
progress    db ".", 0

;**************************************************************************************************
;
;       Includes
;
;**************************************************************************************************
%include    "stdio.asm"
%include    "gdt.asm"
%include    "floppy.asm"
%include    "a20.asm"

;**************************************************************************************************
;
;       main function
;       setups gdt, a20 and loads and executes kernel
;
;**************************************************************************************************
main:
    mov     [bootdrive], ax
    mov     [selfsize], bx

    cli
    xor     ax, ax
    mov     ds, ax
    mov     es, ax

    mov     ax, 0x6000
    mov     ss, ax
    mov     sp, 0xffff
    sti

    mov     si, msg
    call    print16

    call    enable_a20
    call    install_gdt
    sti

    xor     eax, eax
    inc     eax
    add     eax, dword [selfsize]
    
    mov     ebx, addressr
    
    mov     ecx, dword [kernelsize]
    add     ecx, dword [initrdsize]
    
    cmp     ecx, 20
    jnl     reset_remaining
    
    cut_a_bit:
        sub     ecx, 20
        mov     [remaining], ecx
        mov     ecx, 20
        jmp     read
        
    reset_remaining:
        mov     [remaining], word 0
        
    read:
        sti
        push    eax
        push    ecx
        call    read_sectors
        cli

        ; pmode for a moment...
        push    ds
        push    es

        mov     eax, cr0
        or      eax, 1
        mov     cr0, eax
        push    eax
        
        mov     ax, 0x10
        mov     ds, ax
        mov     es, ax
        
        cld
        mov     esi, addressr
        mov     edi, addressp

        mov     eax, dword [sectorsize]
        mul     ecx
        mov     ecx, eax

        cmp     [done], word 0
        je      copy
        
        mov     eax, dword [done]
        mov     ebx, 512
        mul     ebx
        add     edi, eax

    copy:
        rep     movsb

        cmp     [remaining], word 0
        je      endread
        
        ; rmode again, for int 0x13
        pop     eax
        and     al, 0xFE
        mov     cr0, eax
        
        pop     es
        pop     ds

        pop     ecx
        pop     eax

        add     [done], cx
        mov     ecx, dword [remaining]
        
        cmp     ecx, 20
        jnl     reset_remaining
        
        jmp     cut_a_bit
        
    endread:
        cli
        jmp     0x8:stage3

;**************************************************************************************************
;
;       Stage 3 entry point
;       Setups and executes kernel
;
;**************************************************************************************************
bits    32

msg32       db "Kernel and initrd loaded, initializing environment...", 0x0a, 0

stage3:
    mov     ax, 0x10
    mov     ds, ax
    mov     ss, ax
    mov     es, ax
    mov     esp, 0x90000                ; here the stack begin

    call    clear_screen
    call    movcur

    mov     ebx, msg32
    call    print
    
    mov     ebx, [0x10000]
    
    call    addressp