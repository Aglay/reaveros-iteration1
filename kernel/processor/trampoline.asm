;
; Reaver Project OS, Rose License
;
; Copyright (C) 2011-2013 Reaver Project Team:
; 1. Michał "Griwes" Dominiak
; 
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
; 
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
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

global  trampoline_start
global  trampoline_end

extern  ap_initialize

bits    16

trampoline_start:
    inc     byte [cs:24]
    
    cli

    jmp     over
    
    times   16 - ($ - $$)   db  0
    
pml4:   dq 0
flag:   db 0

    times   32 - ($ - $$)   db  0

gdt_start:
    ; null - 0x0
    dd 0
    dd 0

    ; code - 0x8
    dw 0xffff
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

    ; data - 0x10
    dw 0xffff
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0
    
    ; code 64 bit - 0x18
    dw 0
    dw 0
    db 0
    db 10011000b
    db 00100000b
    db 0

    ; data 64 bit - 0x20
    dw 0
    dw 0
    db 0
    db 10010000b
    db 0
    db 0

gdt_end:
    times   80 - ($ - $$)   db  0

gdt:
    dw 39
    dd 0
    
over:
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    add     eax, 16
    mov     ss, ax
    mov     sp, 512
    
    mov     eax, cs
    mov     ebx, 0x10
    mul     ebx
    
    mov     esi, eax
    
    xchg    bx, bx
    
    lea     ebx, [eax + 32]
    mov     dword [82], ebx
    
    lgdt    [80]
    
    add     eax, 512
    
    push    word 0x8
    push    ax
    
    mov     eax, cr0
    or      al, 1
    mov     cr0, eax
    
    retf
    
    times   256 - ($ - $$)  db  0
    
    times   512 - ($ - $$)  db  0
stack_top:

bits    32

pmode:
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    
    mov     esp, esi + 512
    
    mov     eax, cr4
    or      eax, 1 << 5
    mov     cr4, eax
    
    mov     ecx, 0xC0000080
    rdmsr
    or      eax, 1 << 8
    wrmsr
    
    mov     eax, [esi + 16]
    mov     cr3, eax
    
    mov     eax, cr0
    or      eax, 1 << 31
    mov     cr0, eax
    
    push    dword 0x18
    push    ap_initialize
    
    hlt
        
trampoline_end: