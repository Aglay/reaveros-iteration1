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

bits    32

global  _check_long_mode
global  enter_long_mode
global  setup_gdt
global  _setup_idt

_check_long_mode:
    mov     eax, 0x80000000
    cpuid
    cmp     eax, 0x80000001
    jb      .no

    mov     eax, 0x80000001
    cpuid
    test    edx, 1 << 29
    jz      .no

    mov     eax, 1

    ret

    .no:
        xor     eax, eax

        ret

enter_long_mode:
    mov     eax, cr4
    or      eax, 1 << 5
    mov     cr4, eax
    
    mov     ecx, 0xC0000080
    rdmsr
    or      eax, 1 << 8
    wrmsr
    
    mov     eax, cr0
    or      eax, 1 << 31
    mov     cr0, eax
    
    ret
        
setup_gdt:
    pusha
    lgdt    [gdt]
    popa

    jmp     0x10:.ret

    .ret:
    mov     ax, 0x20
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    ret
    
extern  idtr
    
_setup_idt:
    ; disable PIC
    push    eax
    
    mov al, 0xff
    out 0xa1, al
    out 0x21, al

    pusha
    lidt    [idtr]
    popa
    
    sti

    ret

gdt_start:
    ; null:
    dq 0

    ; code 64 bit: 
    dw 0
    dw 0
    db 0
    db 10011000b
    db 00100000b
    db 0

    ; code 32 bit:
    dw 0xffff
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

    ; data:
    dw 0
    dw 0
    db 0
    db 10010000b
    db 0
    db 0

    ; data 32bit:
    dw 0xffff
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt:
    dw gdt_end - gdt_start - 1
    dd gdt_start

%macro isr 1
global  isr%1
isr%1:
    cli
    push    dword 0
    push    dword  %1
    jmp     isrh
%endmacro

%macro isr_err 1
global  isr%1
isr%1:
    cli
    push    dword %1
    jmp     isrh
%endmacro

bits    64

isr 0
isr 1
isr 2
isr 3
isr 4
isr 5
isr 6
isr 7
isr_err 8
isr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr 15
isr 16
isr 17
isr 18
isr 19
isr 20
isr 21
isr 22
isr 23
isr 24
isr 25
isr 26
isr 27
isr 28
isr 29
isr 30
isr 31

extern  isr_handler

isrh:
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi
    
    cli
    call    isr_handler
    sti
    
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax
    
    add     esp, 8
    
    iretq