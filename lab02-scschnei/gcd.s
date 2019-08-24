#=============================================================================
# The gcd(a, b) function recursively computes the greatest common denominator of
# a and b. With gcd(0, 0) being 0.
#
.globl gcd
gcd_start:
        movl    %edi, %eax     # Moves a into %eax
gcd:
        orl     $0, %esi       # Sets zero flag if arg b equals zero
        jnz     gcd_continue   # Compute factorial if nonzero
        jmp     gcd_return     # Return if b is 0

gcd_continue:
        cltd                   # Prepare registers for division by converting
                               # long to double-long
        idivl    %esi          # Divide a by b
        movl    %esi, %eax     # mov b to a
        movl    %edx, %esi     # move remainder of (a / b) to b
        call    gcd            # Make recursive call which will be gcd (b, r)
                               # essentially

gcd_return:
        ret                    # All done, return gcd
