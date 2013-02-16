.global xchg

xchg:
    xorl    %eax, %eax
    movl    4(%esp), %ecx
    movl    8(%esp), %eax
    xchg    %eax, (%ecx)
    ret