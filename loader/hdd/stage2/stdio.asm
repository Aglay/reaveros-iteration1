;
; ReaverOS
; loader/stage2/stdio.asm
; Basic screen routines
;

;
; Reaver Project OS, Rose License
;
; Copyright © 2011-2012 Michał "Griwes" Dominiak
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

;
; print16()
; si - null ended string
;

print16:
    pushad

    .loop:
        lodsb
        or      al, al
        jz      .done
        mov     ah, 0x0e
        int     0x10
        jmp     .loop

    .done:
        popad
        ret

;
; printnum16()
; eax - number to be printed
;

printnum16:
    pushad

    xor     edx, edx

    mov     ebx, dword 10
    div     ebx

    or      eax, eax
    jz      .mod

    call    printnum16

    .mod:
        mov     al, byte '0'
        add     al, dl

        mov     ah, 0x0e
        int     0x10

    popad

    ret

;
; printhex16()
; eax - number to be hex-printed
;

digits:     db "0123456789ABCDEF"

printhex16:
    pushad

    xor     edx, edx

    mov     ebx, dword 16
    div     ebx

    or      eax, eax
    jz      .mod

    call    printhex16

    .mod:
        mov     al, byte [digits + edx]
        mov     ah, 0x0e
        int     0x10

    popad

    ret
