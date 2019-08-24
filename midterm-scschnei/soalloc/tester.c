#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "smallobj.h"


//! Default random number seed.
#define DEFAULT_SEED 242424

#define DEFAULT_OBJECT_SIZE 32
#define DEFAULT_CHUNK_SIZE 1000
#define DEFAULT_TOTAL_OBJS 1000000
#define DEFAULT_LIVE_OBJS 10000


// === TYPES ===================================================================


/*!
 * This type is used to describe an age-distribution of objects that are
 * allocated with the small-object allocator.
 */
typedef struct age_dist_t {
    int weight;    //!< Relative weighting of this age value
    int age;       //!< An age to assign to objects
} age_dist_t;


/*!
 * This struct describes a single allocation in the small-object pool.
 * Each allocation has an age associated with it, which is decremented every
 * round by the tester.  When the age hits 0, the object is released back to
 * the pool.
 */
typedef struct objinfo_t {
    uint8_t *mem;  //!< Pointer to the memory itself
    uint8_t value; //!< Byte value written into the entire memory region
    int age;       //!< Age of the object; when 0, the object is released
} objinfo_t;


// === MODULE-LOCAL STATE ======================================================


//! When true, this turns on verbose output.  Use the "-v" option when running.
static bool verbose = false;

/*!
 * The maximum number of age entries allowed to be specified in the command-line
 * arguments.
 */
#define MAX_AGE_ENTRIES 5

//! A specification of the age distribution of allocated objects.
static age_dist_t age_distribution[MAX_AGE_ENTRIES] = { {100, 10} };

//! The number of entries in the age_distribution array.
static int num_age_entries = 1;

//! The maximum weight to generate for choosing an object age.
static int max_weight = 100;


// === TESTER FUNCTIONS ========================================================


/*!
 * Verify an individual object that has been allocated.
 *
 * The function returns the number of bytes in the object that don't match the
 * expected value.  Ideally this returns 0.
 */
size_t check_object(uint8_t *mem, size_t size, uint8_t value) {
    assert(mem != NULL);

    size_t bad_bytes = 0;
    for (size_t i = 0; i < size; i++) {
        if (mem[i] != value)
            bad_bytes++;
    }

    return bad_bytes;
}


/*!
 * Verify all allocated objects.  Error messages are printed for all issues
 * identified.
 *
 * The function returns the number of objects that fail verification.  Ideally
 * this returns 0.
 */
int check_all_objects(objinfo_t *info, int num_objs, size_t objsize) {
    assert(info != NULL);

    int bad_objects = 0;
    for (int i = 0; i < num_objs; i++) {
        // Skip unallocated objects.
        if (info[i].mem == NULL)
            continue;

        // Verify the contents of allocated objects.
        size_t bad_bytes = check_object(info[i].mem, objsize, info[i].value);
        if (bad_bytes > 0) {
            // TODO:  PRINT ONLY IF VERBOSE
            fprintf(stderr, "ERROR:  Object %p has %zu bad bytes\n",
                info[i].mem, bad_bytes);
            bad_objects++;
        }
    }

    if (bad_objects > 0)
        fprintf(stderr, "ERROR:  Found %d bad objects\n", bad_objects);

    return bad_objects;
}


/*!
 * Generates a uniformly distributed random value in the range
 * [min_val, max_val].  The maximum value must be at least the minimum value.
 */
int rand_int(int min_val, int max_val) {
    assert(max_val >= min_val);

    if (min_val == max_val)
        return min_val;

    return min_val + random() % (max_val - min_val + 1);
}


/*! Generates a new object age based on the age-distribution specification. */
int make_object_age(void) {
    int age_idx = rand_int(0, max_weight - 1);
    for (int i = 0; i < num_age_entries; i++) {
        if (age_idx < age_distribution[i].weight)
            return age_distribution[i].age;

        age_idx -= age_distribution[i].weight;
    }
    // Shouldn't get here, unless the max_weight value is wrong or something.
    assert(false);
}


void run_test(size_t objsize, int total_objs, int max_live_objs,
    smallobj_pool_t *pool) {

    assert(total_objs > 0);
    assert(max_live_objs > 0);
    assert(max_live_objs <= total_objs);

    objinfo_t *info = malloc(sizeof(objinfo_t) * max_live_objs);
    if (!info) {
        fprintf(stderr, "ERROR:  Couldn't allocate object-info array for %d "
            "live objects\n", max_live_objs);
        abort();
    }

    bzero(info, sizeof(objinfo_t) * max_live_objs);

    int objects_allocated = 0;     // Total objects allocated
    int objects_released = 0;      // Total objects released
    int live_objects = 0;          // Number of objects currently "live"

    // Total amount of bytes currently required by the small-object pool
    size_t initial_pool_size = total_pool_size(pool);

    size_t pool_high_watermark = initial_pool_size;
    size_t pool_avg_size = 0;

    printf("Initial pool size:  %zu bytes\n\n", initial_pool_size);

    int round_no = 0;
    while (objects_released < total_objs) {
        round_no++;

        // Iterate through all object-info entries.  Decrease the age of each
        // live object.  If any hits an age of 0, release it.

        int released_this_round = 0;
        for (int i = 0; i < max_live_objs; i++) {
            // Skip entries that are currently empty.
            if (info[i].mem == NULL)
                continue;

            assert(info[i].age > 0);

            info[i].age--;
            if (info[i].age == 0) {
                so_free(pool, info[i].mem);

                // Clear the information about this allocation.
                info[i].mem = NULL;
                info[i].value = 0;

                // Update bookkeeping variables.
                live_objects--;
                objects_released++;
                released_this_round++;
            }
        }

        if (released_this_round > 0) {
            // Verify all live objects after releasing.
            if (check_all_objects(info, max_live_objs, objsize) > 0) {
                fprintf(stderr,
                    "ERROR:  Corruption detected after releasing objects\n");
                abort();
            }
        }

        // If we have space in our object-info array, and if we still haven't
        // allocated all the objects we intended to, allocate some new objects
        // for the pool.
        int allocated_this_round = 0;
        if (objects_allocated < total_objs && live_objects < max_live_objs) {
            int to_allocate = rand_int(1, max_live_objs - live_objects);

            // Don't exceed the maximum number of objects to create.
            if (to_allocate > total_objs - objects_allocated)
                to_allocate = total_objs - objects_allocated;

            allocated_this_round = to_allocate;

            for (int i = 0; i < max_live_objs && to_allocate > 0; i++) {
                // Skip entries that are currently in use.
                if (info[i].mem != NULL)
                    continue;

                // Found an available entry that can record an allocation.

                // This allocation may fail if a new chunk needs to be allocated
                // but malloc() can't do it.
                info[i].mem = so_alloc(pool);
                if (info[i].mem == NULL) {
                    fprintf(stderr, "ERROR:  small-object allocation failed\n");
                    abort();
                }

                // Assign an age.

                info[i].age = make_object_age();
                assert(info[i].age > 0);

                // Write a byte value between 1 (0x01) and 200 (0xC8) inclusive,
                // into the newly allocated memory region.
                info[i].value = (uint8_t) rand_int(1, 200);
                memset(info[i].mem, info[i].value, objsize);

                // Record that we have one more allocated object.
                objects_allocated++;
                live_objects++;
                to_allocate--;
            }

            assert(live_objects <= max_live_objs);
            assert(objects_allocated >= objects_released);
            assert(objects_allocated <= total_objs);
        }

        if (allocated_this_round > 0) {
            // Verify all live objects after allocating.
            if (check_all_objects(info, max_live_objs, objsize) > 0) {
                fprintf(stderr,
                    "ERROR:  Corruption detected after allocating objects\n");
                abort();
            }
        }

        if (released_this_round > 0 || allocated_this_round > 0) {
            // Look at the current pool size.
            size_t current_pool_size = total_pool_size(pool);
            pool_avg_size += current_pool_size;
            if (current_pool_size > pool_high_watermark)
                pool_high_watermark = current_pool_size;

            if (verbose) {
                // Print out details of what happened this round.
                printf("Round %d:\t-%d\t+%d\tSize = %zu\tHWM = %zu\n", round_no,
                    released_this_round, allocated_this_round,
                    current_pool_size, pool_high_watermark);
            }
        }
    }

    // Compute average pool size over the entire run.
    pool_avg_size /= round_no;

    printf("Total rounds:  %d\n", round_no);
    printf("Average pool size:  %zu bytes\n", pool_avg_size);
    printf("Pool high-watermark:  %zu bytes\n", pool_high_watermark);

    size_t final_pool_size = total_pool_size(pool);
    printf("Final pool size:  %zu bytes\n", final_pool_size);

    if (final_pool_size != initial_pool_size) {
        fprintf(stderr, "ERROR:  Final pool size should be %zu bytes; "
            "something bad happened\n", initial_pool_size);
    }
}


/*! Prints out program usage details. */
void usage(const char *prog) {
    printf("usage: %s [options]\n", prog);
    printf("\tRuns the small-object allocator tester with the specified options.\n\n");

    printf("-v | --verbose\n");
    printf("\tTurns on verbose output during testing.\n\n");

    printf("-s <seed> | --seed <seed>\n");
    printf("\tSpecifies the random number seed for testing.  Default is %d.\n\n",
        DEFAULT_SEED);

    printf("-S <objsize> | --objsize <objsize>\n");
    printf("\tSpecifies the size of objects in the small-object pool.  Default is %d.\n\n",
        DEFAULT_OBJECT_SIZE);

    printf("-C <chunksize> | --chunksize <chunksize>\n");
    printf("\tSpecifies the number of objects in each chunk of the small-object pool.\n"
           "\tDefault is %d.\n\n", DEFAULT_CHUNK_SIZE);

    printf("-T <count> | --total_objs <count>\n");
    printf("\tSpecifies the total number of objects to create in the test run.\n"
           "\tDefault is %d.\n\n", DEFAULT_TOTAL_OBJS);

    printf("-L <count> | --live_objs <count>\n");
    printf("\tSpecifies the maximum number of live objects to have at any time\n"
           "\tduring the test run.  Default is %d.\n\n", DEFAULT_LIVE_OBJS);

    printf("-A <age-spec> | --ages <age-spec>\n");
    printf("\tSpecifies the ages that may be assigned to objects, along with their\n"
           "\trelative weightings.  The format is \"weight:age[,weight:age,...]\".\n\n"
           "\tFor example, -A \"90:10,10:200\" specifies that 90%% of objects will\n"
           "\thave an age of 10, and 10%% of objects will have an age of 200.\n"
           "\tTo give all objects the same age, specify just one entry with any\n"
           "\tweight and the desired age, e.g. -A \"1:10\" will cause all objects\n"
           "\tto have an age of 10.\n\n"
           "\tA maximum of %d weight:age pairs may be specified.\n\n", MAX_AGE_ENTRIES);
}


/*! Parses the age-specification string. */
void parse_ages(const char *agespec) {
    int i = 0;
    char *endp;
    long weight, age;

    num_age_entries = 0;
    max_weight = 0;
    while (true) {
        // First comes the weight
        weight = strtol(agespec, &endp, 10);
        if (agespec == endp) {
            fprintf(stderr, "ERROR:  Couldn't parse weight at index %d\n", i);
            exit(1);
        }

        if (weight <= 0) {
            fprintf(stderr, "ERROR:  Weight at index %d is not a positive number\n", i);
            exit(1);
        }

        if (*endp == '\0') {
            // Reached end of sequence...?!
            fprintf(stderr, "ERROR:  weight must be followed by an age\n");
            exit(1);
        }
        else if (*endp != ':') {
            fprintf(stderr, "ERROR:  weight and age must be separated by ':' character\n");
            exit(1);
        }

        // Move to the age part of the spec
        agespec = endp + 1;
        age = strtol(agespec, &endp, 10);
        if (agespec == endp) {
            fprintf(stderr, "ERROR:  Couldn't parse age at index %d\n", i);
            exit(1);
        }

        if (age <= 0) {
            fprintf(stderr, "ERROR:  Age at index %d is not a positive number\n", i);
            exit(1);
        }

        // Store the weight:age pair.
        age_distribution[i].weight = weight;
        age_distribution[i].age = age;

        num_age_entries++;
        max_weight += weight;

        if (*endp == '\0') {
            // Reached end of sequence.
            break;
        }
        else if (*endp != ',') {
            fprintf(stderr, "ERROR:  weight:age pairs must be separated by ',' character\n");
            exit(1);
        }

        agespec = endp + 1;

        // Go on to the next index.
        i++;
        if (i >= MAX_AGE_ENTRIES) {
            fprintf(stderr, "ERROR:  may only specify up to %d weight:age entries\n",
                MAX_AGE_ENTRIES);
            exit(1);
        }
    }
}


/*! Program entry-point. */
int main(int argc, char **argv) {
    size_t objsize = DEFAULT_OBJECT_SIZE;
    int objects_per_chunk = DEFAULT_CHUNK_SIZE;

    int total_objs = DEFAULT_TOTAL_OBJS;
    int max_live_objs = DEFAULT_LIVE_OBJS;

    unsigned int seed = DEFAULT_SEED;

    int ch;
    while (true) {
        int option_index = 0;
        struct option longopts[] = {
            {"seed",       required_argument, 0, 's'},
            {"objsize",    required_argument, 0, 'S'},
            {"chunksize",  required_argument, 0, 'C'},
            {"total_objs", required_argument, 0, 'T'},
            {"live_objs",  required_argument, 0, 'L'},
            {"ages",       required_argument, 0, 'A'},
            {"verbose",    no_argument,       0, 'v'},
            {0,            0,                 0,  0 }
        };

        ch = getopt_long(argc, argv, "s:S:C:T:L:A:v", longopts, &option_index);
        if (ch == -1)
            break;

        switch (ch) {
        case 's':
            seed = atoi(optarg);
            break;

        case 'S':
            objsize = atoi(optarg);
            if (objsize < (int) sizeof(intptr_t)) {
                fprintf(stderr, "ERROR:  object size must be at least %zu\n",
                    sizeof(intptr_t));
                exit(1);
            }
            break;

        case 'C':
            objects_per_chunk = atoi(optarg);
            if (objects_per_chunk < 100) {
                fprintf(stderr,
                    "ERROR:  must have at least 100 objects per chunk\n");
                exit(1);
            }
            break;

        case 'T':
            total_objs = atoi(optarg);
            if (total_objs < 1) {
                fprintf(stderr,
                    "ERROR:  must have at least 1 total object\n");
                exit(1);
            }
            break;

        case 'L':
            max_live_objs = atoi(optarg);
            if (max_live_objs < 1) {
                fprintf(stderr,
                    "ERROR:  must have at least 1 live object\n");
                exit(1);
            }
            break;

        case 'A':
            // This function will exit the program if a parse error is detected.
            parse_ages(optarg);
            break;

        case 'v':
            verbose = true;
            break;

        case '?':
            usage(argv[0]);
            exit(0);
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", ch);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "ERROR:  unhandled arguments");
        while (optind < argc)
            fprintf(stderr, "  %s", argv[optind++]);
        fprintf(stderr, "\n");
        exit(1);
    }

    printf("Small-Object Pool Tester:\n");

    printf(" * objects are %zu bytes; chunks hold %d objects\n", objsize,
        objects_per_chunk);

    printf(" * %d total objects, %d max live at any given time\n",
        total_objs, max_live_objs);

    printf(" * random seed is %u\n", seed);

    printf(" * %d weight:age entries\n", num_age_entries);
    for (int i = 0; i < num_age_entries; i++) {
        printf("\tweight:  %d\tage:  %d\n", age_distribution[i].weight,
            age_distribution[i].age);
    }

    printf("\n");

    srandom(seed);

    smallobj_pool_t *pool = make_so_pool(objsize, objects_per_chunk);
    if (!pool) {
        fprintf(stderr, "ERROR:  cannot allocate small-object pool!");
        abort();
    }

    printf("Running test\n\n");
    run_test(objsize, total_objs, max_live_objs, pool);

    printf("Releasing small-object pool\n");
    release_so_pool(pool);

    return 0;
}
