BITS 64

mov rcx, [rdi]
cmp dword [edi], 0
add dword [edi], dword 0x1
sub dword [edi], dword 0x1
test rcx, rcx

; lea rax,[rel end]
; lea rbx,[rel end]
je   $ + 0x1
jmp  0x14

add qword [rdi], 2147483646
sub qword [rdi], 2147483646


end:
