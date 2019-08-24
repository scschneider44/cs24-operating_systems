# perflab
## Q1
       Clock speed decreases as the array list size increases. This is because
       the array list is initialized with size 16. This means smaller arrays
       time per element appended will be dominated by the O(n) operation of
       copying over the array to an array with a larger capacity. As the arrays
       increase in size the constant time append dominates the runtime as there
       are relatively few resizing operations relative to appending operations
       because the resizing multiplies the capacity by 2 and powers of 2
       increase rapidly.
## Q3
       My realloc optimization doesn't show improvement which was a little
       surprising. This is most likely because there isn't a lot of contiguous
       data within the cloud. This means when realloc was called, it was simply
       doing a memcpy to a new destination in memory. Realloc does this when
       there isn't room to just expand the existing memory space. Ideally, there
       would be space in memory to simply expand the existing array. This would
       make realloc faster because no copying of data would have to take place
       which is O(n). Instead realloc would just malloc more memory and return
       the same pointer which should be faster than copying the memory. However,
       it is possible that what I suggested isn't the case and the reason the
       optimization is performing similar to the unoptimized code is because
       realloc calls malloc and that is much slower than the copying of data.
       I'm not sure exactly which case it is. The behavior is the similar to
       the unoptimized run regardless of array size.
## Q5
       Memmove() exhibits an approximately 8x improvement over manually shifting
       the elements. This is because memmove() utilizes the cache to load, copy
       and move more than one int over at a time. This is an example of SIMD
       which is Single Instruction Multiple Data which is pretty self
       explanatory. There are also certain machine code instructions that can be
       used to speed up this process when it is known that data is being moved.
       Memmove() could use this but the manual process definitely will not.
## Q8
       The small object pool linked lists perform better than the linked list
       that uses malloc for both insert and append. This makes sense because
       most of the improvement comes from reduction in malloc overhead rather
       than locality. We know this because when we limit the small object pool
       linked list to one object per cache line we only get a slight worsening
       in performance. Thus, the reason malloc isn't called every time a node is
       created is the main reason the small object pool is better. Locality only
       improves performance a little. The small object pool linked lists perform
       worse with very small lists likely due to the malloc overhead when
       allocating the pool and chunk at the beginning. This overhead quickly
       gets dominated by the lack of such after the chunk has been malloc'd.
## Q9
       The shape of the curves for both the optimized and unoptimized linked
       list are roughly the square root curve. There are, however, some
       interesting features. The slow accent at the beginning is because in the
       caching process some of the nodes are cached due to luck essentially but
       others aren't so the process has to go to L3 or further to find the node.
       Thus, the increase in speed relates to the fact that as the list grows
       the less likely it is to get lucky and have the next node in the cache.
       The plateau relates L3 cache. When the list gets too big the chance that
       the next node is in L1 or L2 is near 0 but the chance that it's in L3 is
       pretty high because L3 is very large compared to L1 and L2. Thus, this
       plateau is approximately the clock speed of accessing something in L3.
       Eventually the list gets so big that the chance that the next node is
       luckily in L3 is unlikely to we see a large spike that is associated with
       the search for the node within the DRAM which is much slower than the
       SRAM. The optimized linked list performs better and peaks lower and later
       because, as mentioned, it has better spatial locality. This means there
       is more than luck involved that the next node is in a cache rather than
       DRAM.
## Q10
       As we can see, array lists are better at both accessing elements of a
       list and iterating through the elements. because the data has locality.
       During iteration, cache misses are limited because the data surrounding
       the element being accessed gets brought into the cache so accessing the
       next element during iteration is much faster. For access, an array list
       can access memory without iteration thanks to the locality of the data.
       Because all the data is adjacent, using the pointer to the front of the
       list we can access it because we know its relation to the front of the
       list. Thus, if your program requires list access then array lists should
       be used.

       If there are a large amount of inserts and/or appends AND random list
       access and/or iteration aren't necessary for the program than a linked
       list should be used. This is because inserts and appends for a linked
       list don't require copying and moving data which makes it much faster.
       However, list iteration, and thus access, is very slow so if list access
       is needed, array lists should be used as noted.

# Extra Credit
## EC1
       It doesn't make a difference. This makes sense because -O3 doesn't
       optimize that much more than -O2. It just does more subtle space-time
       trade off optimizations. Thus, the effect is not very big.
## EC2
       There is a sawtooth pattern around powers of two in these graphs because
       at powers of two greater than 16 is when the array is resized. This
       resizing is expensive. So if an array is only slightly larger than a
       power of two, that costly operation of resizing hasn't had a chance to
       be amoritized to an O(1) operation so we see a slight increase in average
       clocks per second.       
## EC3
       Write your answer here.
