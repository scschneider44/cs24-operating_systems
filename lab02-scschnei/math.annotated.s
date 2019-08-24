/* This file contains x86-64 assembly-language implementations of three
 * basic, very common math operations.
 *
 * The common theme in these operations is that there are no conditional jump
 * operations. This means there is no branching. The code in these three
 * examples use conditional moves, or conditional transfers of data, rather than
 * conditional jumps, also known as transfer of control. A compiler wants to
 * avoid conditional jumps because * they decrease efficiency because it is
 * harder for the processor to effectively pipeline.
 */

        .text

/*====================================================================
 * This function takes in two integers, x and y, and returns the integer that
 * is smallest. Thus, this performs the simple operation min(x, y).
 * int f1(int x, int y) {
 *    if (y > x) {
 *      y = x;
 *    }
 *    return y;
 * }
 */
.globl f1
f1:
        movl    %edi, %edx    # Move x into register %edx
        movl    %esi, %eax    # Move y into register %eax
        cmpl    %edx, %eax    # Compare y and x --> y ? x
        cmovg   %edx, %eax    # if y > x move x into %eax
        ret                   # return eax


/*====================================================================
 * This function takes an integer, x, and returns the absolute value of x.
 * Thus, this function is abs(x).
 * int f2(int x) {
 *    int x1 = x;
 *    int x2 = x >> 31;   // x2 is the sign bit
 *    // This XOR with the sign bit will return x1 if the number is positive,
 *    // 0 if the number is 0, and abs(x1)-1 if the number is negative.
 *    x1 = x1 ^ x2;
 *    // in the positive case this will be x - 0 = x. In the case x is 0 it will
 *    // be 0 - 0 = 0, and in the negative case this will be
 *    // (abs(x1) - 1) - (-1) = abs(x1)
 *    x1 -= x2;
 *    return x1;
 * }
 *
 */
.globl f2
f2:
        movl    %edi, %eax    # Move x into register %eax
        movl    %eax, %edx    # Move x into register %edx from %eax
        sarl    $31, %edx     # Get the sign bit of x and put it in %edx
        xorl    %edx, %eax    # XOR x with the sign bit of x and store the
                              # the result in %eax
        subl    %edx, %eax    # Subtract the sign bit from the result from the
                              # XOR and store it in %eax
        ret                   # return eax


/*====================================================================
 * This function returns the sign of an integer, x. It returns 0 if x is 0.
 * Thus, this performs sign(x).
 * int f3(int x) {
 *    int x1 = x;
 *    int x2 = x >> 31;   // gets sign bit of x
 *    x1 = 1;
 *    if (x1 > 0) {
 *        x2 = x1
 *        return x2;      // This will be 1
 *    }
 *    return x2;          // This will be the sign bit, but either -1 or 0
 * }
 *
 */

.globl f3
f3:
        movl    %edi, %edx    # Move x into register %edx
        movl    %edx, %eax    # Move x into register %eax
        sarl    $31, %eax     # Find the sign bit of x via left shit and put it
                              # in %eax
        testl   %edx, %edx    # Test if x = 0
        movl    $1, %edx      # Move 1 into register into %edx
        cmovg   %edx, %eax    # If x > 0, move 1 into %eax
        ret                   # return %eax
