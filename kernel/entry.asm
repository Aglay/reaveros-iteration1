;
; ReaverOS
; kernel/entry.asm
; Kernel's entry point.
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

bits    64

global  entry
extern  kernel_main

entry:
    mov     rax, 0xFFFFFFFF80000000 + 16
    jmp     rax

times 16 - ($-$$) db 0

highmemory:
    push    rbp
    mov     rbp, rsp

    add     rsp, 8

    mov     rdi, qword [rsp]
    mov     rsi, qword [rsp + 8]
    mov     edx, dword [rsp + 16]
    mov     rcx, qword [rsp + 20]
    mov     r8, qword [rsp + 28]

    jmp     kernel_main