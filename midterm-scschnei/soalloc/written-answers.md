**(a)** Explain in detail why the initial version of the small-object allocator
       provided to you exhibits this particular behavior. Aim for completeness,
       clarity and conciseness in your answer.

       The initial version exhibits the particular behavior because when a small
       object is freed it doesn't actually free up space until the whole chunk
       is empty. This means when there is a relatively small portion of
       long-lived objects, entire large chunks end up holding a small amount of
       long-lived objects that are surrounded by garbage, the space that used to
       hold the short-lived objects, that cannot be freed due to the existence
       of the long-lived objects. This leads to memory inefficiency as we see
       because the long-lived objects make it such that more large chunks are
       created despite the existing chunks not actually holding very many
       objects because the chunks holding the long-lived objects appear full
       during the allocation process.





**(b)**  Describe a modification that can be made to the internals of the
         small-object allocator to resolve this problem, so that the space
         required by the pool is no longer affected by variations in the lifetimes
         of small objects. Your description should be at a level of detail that
         another programmer could take your description and implement it against
         the code on their own.

         To fix this, when we are checking for a nonfull chunk in
         get_nonfull_chunk we first check if there is room in the previous
         sense, that the next available index is less than
         pool->objects_per_chunk. If the index is indeed less than the possible
         number of objects then we allocate to this chunk like we did before,
         we pass this chunk to alloc_object_from_chunk and return the address
         that is objsize * next_available away from the beginning of the chunk.
         If this isn't true then we check if the number of free spots within the
         chunk is greater than 0. If it is we pass this chunk to
         alloc_object_from_chunk. In alloc_object_from_chunk it will see that
         there is no space available in the previous sense with an if statement.
         So, we then iterate through the objects in the chunk and if it contains
         garbage (0xEE in the code given) then we return that space in memory to
         so_alloc to be filled by the new data to be put in memory. This means
         that the high-watermark won't exceed ~320 kb because we fill in any
         empty space available, not just space where the "next available" spot
         is. This means we will never need more than
         (max num objects / num objects per chunk) number of chunks. Allocation
         remains constant time because the time to allocated to a previously
         freed space is dependent only on the size of the chunk, not the number
         of objects.
