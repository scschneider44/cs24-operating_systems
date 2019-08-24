# This function takes in the memory to write to and the size of the message
# and makes a system call to write. It does this by filling in the correct
# arguments into the registers that constitute the arguments for system calls
# which is different than the normal registers for functions.

.globl output
output:
        movq   %rsi, %rdx    # move first argument to the 3rd argument of a
                             # system call
        movq   %rdi, %rsi    # move the second argument to the second argument
                             # of a system call
        movq   $1, %rdi      # Move 1 into the first argument as specified
        movq   $1, %rax      # call the write system call with 1.
        syscall
        ret
