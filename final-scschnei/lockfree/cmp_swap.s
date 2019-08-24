# This function will implement the equivalent c code
# int compare_and_swap(uint32_t *target, uint32_t old, uint32_t new) {
#     if (*target == old) {
#         *target = new
#         return 1;
#     }
#     return 0;
# }
#
# In assembly the first argument passed into functions goes into register %rdi.
# The second argument goes into register %rsi and the third argument goes into
# register %rdx. Thus the pointer to our target will be in rdi, the old value
# will be in rsi and the new value will be in rdx. This function moves the old
# value into %rax where it will be compared to. It then compares the value
# target is pointing to to rax (which is the old value) and if they're the same
# then rdx, which is the new value, gets passed into the memory target is
# poiting to. If *target and old weren't equal then we put 0 into rax
# and jump to return. Otherwise, we put 0 in rax then 1 in rax and then return.

.globl compare_and_swap
compare_and_swap:
            movq      %rsi, %rax   # make the old val the value to compare with
            lock                   # lock to make this exchange atomic
            cmpxchg   %rdx, (%rdi) # make the new value the *target if *target
                                   # equals the old value
            movq      $0,  %rax    # move 0 into the return register
            jnz       cmp_return   # if the exchange didn't happen return 0
            movq      $1,  %rax    # if the exchange happened put 1 in rax
                                   # to be returned
cmp_return:
            ret
