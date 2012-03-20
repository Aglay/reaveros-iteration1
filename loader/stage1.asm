;
; ReaverOS
; loader/stage1.asm
; First stage bootloader
;

;
; Reaver Project OS, Rose License
;
; Copyright (C) 2011-2012 Reaver Project Team:
; 1. Michał "Griwes" Dominiak
; 
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
; 
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, adn to alter it and redistribute it
; freely, subject to the following restrictions:
; 
; 1. The origin of this software must not be misrepresented; you must not
;    claim that you wrote the original software. If you use this software
;    in a product, an acknowledgment in the product documentation is required.
; 2. Altered source versions must be plainly marked as such, and must not be
;    misrepresented as being the original software.
; 3. This notice may not be removed or altered from any source distribution.
; 
; Michał "Griwes" Dominiak
; 

bits    16
org     0x0

entry:
    jmp     stage1

times 8 - ($-$$) db 0

;
; Reaver Floppy Header
; Note: there is no BPB here. This bootloader will work on both floppy
; (useless feature) and HDD supporting int 0x13 extensions. For El-Torito
; bootloader, see eltorito.asm.
;

magic:          dd 0xFEA7EFF5           ; REAVERFS
sector_size:    dw 512
num_sectors:    dw 2880
sec_per_track:  dw 18
heads_per_cyl:  dw 2
reserved:       dw 0                    ; filled by floppy creator - 21, 22 byte
stage2_size:    dw 0                    ; filled by floppy creator - 23, 24 byte
starting:       dq 0                    ; filled by hdd installer

;
; Variables
;

packet:
    .size:      db 0x10
    .zero:      db 0
    .number:    dw 0
    .poffset:   dw 0
    .pseg:      dw 0x07e0               ; hardcoded segment for second stage bootloader
    .exstart:   dq 0

msg:            db "Loading...", 0x0a, 0x0d, 0
progress:       db ".", 0
error:          db " Error", 0
bootdrive:      db 0
sector:         db 0
head:           db 0
track:          db 0

;
; print()
; si - null terminated string
;

print:
    .start:
        lodsb
        or      al, al
        jz      .done
        mov     ah, 0x0e
        int     0x10
        jmp     .start
    .done:
        ret

;
; convert_address()
; ax - linear address
;

convert_address:
    ; just some head/sector/track magic
    xor     dx, dx
    div     word [sec_per_track]
    inc     dl
    mov     byte [sector], dl
    xor     dx, dx
    div     word [heads_per_cyl]
    mov     byte [head], dl
    mov     byte [track], al
    ret

;
; read_sectors_basic()
; Reads from bootdrive using legacy int 0x13 functions
; 
; ax - starting sector
; cx - number of sectors
; es:bx - buffer
;

read_sectors_basic:
    .begin:
        mov     di, 0x5
    .loop:
        push    ax
        push    bx
        push    cx
        
        call    convert_address

        mov     ah, 0x02
        mov     al, 0x01

        mov     ch, byte [track]
        mov     cl, byte [sector]

        mov     dh, byte [head]
        mov     dl, byte [bootdrive]

        int     0x13

        jnc     .end

        xor     ax, ax
        int     0x13

        dec     di

        pop     cx
        pop     bx
        pop     ax

        jnz     .loop

        mov     si, error
        call    print

        cli
        hlt

    .end:
        mov     si, progress
        call    print

        pop     cx
        pop     dx
        pop     ax

        add     bx, word [sector_size]
        inc     ax

        loop    .begin

        ret

;
; check_extended_0x13()
; return: ax - 0 if not present
;

check_extended_0x13:
    xor     ax, ax

    mov     dl, [bootdrive]

    cmp     dl, 0
    je      .floppy
    cmp     dl, 1
    je      .floppy

    mov     ah, 0x41
    mov     bx, 0x55aa

    int     0x13

    jnc     .return

    .no:
        xor     ax, ax
        mov     ah, 0x8
        int     0x13

        shr     dx, 8
        inc     dx
        mov     word [heads_per_cyl], dx
        
        xor     ch, ch
        and     cl, 0x3f
        mov     word [sec_per_track], cx

    .floppy:
        xor     ax, ax
        ret

    .return:
        mov     ax, 1
        ret

;
; read_sectors_extended()
; Warning: high level of function awesomeness included.
; qword [packet.exstart] - starting sector
; cx - number of sectors
;

read_sectors_extended:
    mov     word [packet.number], cx
    
    mov     ah, 0x42
    mov     dl, [bootdrive]
    mov     si, packet

    int     0x13

    jc      .error

    ret

    .error:
        mov     si, error
        call    print

        cli
        hlt

;
; Entry point
;

stage1:
    cli

    mov     ax, 0x07c0
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    xor     ax, ax
    mov     ss, ax
    mov     sp, 0x7c00
    sti

    xor     ax, ax
    mov     [bootdrive], dl

    mov     si, msg
    call    print

    call    check_extended_0x13
    cmp     ax, 0

    jne     .extended

    mov     ax, 0x07e0
    mov     es, ax
    xor     bx, bx

    mov     cx, word [reserved]
    dec     cx
    mov     ax, word [starting]
    inc     ax

    call    read_sectors_basic

    jmp     .jump

    .extended:
        mov     cx, word [reserved]
        dec     cx
        mov     qword [packet.exstart], qword [starting]
        inc     qword [packet.exstart]

        call    read_sectors_extended

    .jump:
        push    word [bootdrive]
        mov     cx, word [reserved]
        dec     cx
        push    cx

        mov     ax, [reserved]
        sub     ax, [stage2_size]
        push    ax

        push    qword [starting]

        push    word 0x07e0
        push    word 0x0000

        mov     si, progress
        call    print

        retf

;
; Boot signature
;

times   510 - ($-$$) db 0
dw      0xaa55