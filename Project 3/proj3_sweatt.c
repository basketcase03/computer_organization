/* @file proj3_sweatt.c
 * @brief Main program for the cache simulator.
 * @author Julian L. Sweatt
 */

/*----------------------------------*
 *             IMPORTS              *
 *----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*----------------------------------*
 *             CONFIG               *
 *----------------------------------*/
#define DEBUG_MODE 0
#define MAX_LINES 100

/*----------------------------------*
 *              HEADER              *
 *----------------------------------*/
// ---------- Structures ---------- //
/**
 * @struct Line
 * @brief Structure intended to hold a line of input containing an unsigned integer and a single character R or W.
 */
typedef struct
{
    char operation;              // Indicates (R)ead or (W)rite Operation
    unsigned int address;        // Address to Cache
} Line;

/**
 * @struct LineList
 * @brief List of lines.
 */
typedef struct
{
    Line * lines;
    unsigned int size;
} LineList;

/**
 * @struct Block
 * @brief Block within a set;
 */
typedef struct
{
    int address;
    char dirty;
    int lastused;
} Block;

/**
 * @struct Set
 * @brief Set within a cache;
 */
typedef struct
{
    Block * blocks;
} Set;

/**
 * @struct Cache
 * @brief General purpose cache.
 */
typedef struct
{
    Set* sets;
    unsigned int hits;
    unsigned int misses;
    unsigned int memrefs;
    unsigned int cacheReferences;
} Cache;

// ---- Dynamic Input Functions --- //
/**
 * @brief Initialize a dynamic list of lines.
 * @return void
 */
void initLines(void);

/**
 * @brief Deinitialize a dynamic list of lines.
 * @return void
 */
void deinitLines(void);

/**
 * @brief Append a line to a list of lines dynamically.
 * @param char operation Indicates (R)ead or (W)rite Operation.
 * @param int address Address portion of line.
 * @return void
 */
void addLine(char operation, int address);

/**
 * @brief Print the list of lines to stdout.
 * @return void
 * @private This is a debug function.
 */
void printLines(void);

/**
 * @brief Print block size, number of sets, associativity, and list of lines from input.
 * @return void
 * @private This is a debug function.
 */
void printInput(void);

// -------- Cache Functions ------- //
/**
 * @brief Initialize a set associative cache.
 * @return void
 */
void initCache(void);

/**
 * @brief Deinitialize the set associative cache.
 * @return void
 */
void deinitCache(void);

/**
 * @brief Reset the set associative cache's contents.
 * @return void
 */
void resetCache(void);

/**
 * @brief Calculate bitwise breakdown of the address.
 * @return void
 */
void calculateAddressBits(void);

/**
 * @brief Extract the tag bits from an address.
 * @param unsigned int address Address to extract from.
 * @return unsigned int Tag bits.
 */
unsigned int getTagBits(unsigned int address);

/**
 * @brief Extract the index bits from an address.
 * @param unsigned int address Address to extract from.
 * @return unsigned int Index bits.
 */
unsigned int getIndexBits(unsigned int address);

/**
 * @brief Extract the offset bits from an address.
 * @param unsigned int address Address to extract from.
 * @return unsigned int Offset bits.
 */
unsigned int getOffsetBits(unsigned int address);

/**
 * @brief 
 * @param Line* l Line to replace in cache.
 * @param int lineNum Current line number.
 * @return int Replacement index.
 */
int lruReplace(Line* l, int lineNum);

/**
 * @brief Cache a line.
 * @param Line* l Line to add to the cache.
 * @param int lineNum Line number to cache.
 * @param cachingMethod char Method of caching to use, Write (B)ack or Write (T)hrough.
 * @return void
 */
void cacheLine(Line* l, int lineNum ,char cachingMethod);

/**
 * @brief Simulate caching instructions using a particualr method.
 * @param char cachingMethod Method of caching to use, Write (B)ack or Write (T)hrough.
 * @return void
 */
void simulate(char cachingMethod);

/**
 * @brief Print the shared and basic information of the set associative cache.
 * @return void
 */
void printHeader(void);

/**
 * @brief Print the cache report after processing.
 * @param char cachingMethod Method of caching to report Write (B)ack or Write (T)hrough.
 * @return void
 */
void printCacheReport(char cachingMethod);

/**
 * @brief Print the set associative cache's contents.
 * @return void
 * @private This is a debug function.
 */
void printCache(void);

/**
 * @brief Print translated line addresses.
 * @return void
 * @private This is a debug function.
 */
void printTranslatedLines(void);

// ----------- Utilities ---------- //
/**
 * @brief Parse input from stdin. Expects 3 integers, each on seperate lines, followed by
 *        an indefinite list of W/R address lines (char and int).
 * @return void
 * @private This is a debug function.
 */
void parseInput(void);

/**
 * @brief Calculate the log with explicit base.
 * @param int num The number to log.
 * @param int base The base of the log.
 * @return void
 */
double logBase(int num, int base);

/*----------------------------------*
 *             Globals               *
 *----------------------------------*/
LineList * LINE_LIST;
Cache* CACHE;
unsigned int BLOCK_SIZE;
unsigned int NUM_SETS;
unsigned int SET_ASSOCIATIVITY;
unsigned int OFFSET_BITS;
unsigned int INDEX_BITS;
unsigned int TAG_BITS;

/*----------------------------------*
 *          IMPLEMENTATIONS         *
 *----------------------------------*/
void initLines(void)
{
    LINE_LIST = (LineList*) malloc(sizeof(LineList));
    LINE_LIST->lines = (Line*) malloc(sizeof(Line));
    LINE_LIST->size = 0;
}

void deinitLines(void)
{
    free(LINE_LIST->lines);
    free(LINE_LIST);
}

void addLine(char operation, int address)
{
    LINE_LIST->size += 1;
    LINE_LIST->lines = (Line*)realloc(LINE_LIST->lines, sizeof(Line)*LINE_LIST->size);
    LINE_LIST->lines[LINE_LIST->size-1].address = address;
    LINE_LIST->lines[LINE_LIST->size-1].operation = operation;
}

void printLines(void)
{
    int i;
    for(i = 0; i < LINE_LIST->size; i++)
    {
        printf("%c %d\n", LINE_LIST->lines[i].operation, LINE_LIST->lines[i].address);
    }
}

void parseInput(void)
{
    // Get Base Variables
    scanf("%d", &BLOCK_SIZE);
    scanf("%d", &NUM_SETS);
    scanf("%d\n", &SET_ASSOCIATIVITY);

    // Indefinite Line Input
    char lineBuffer[256];
    char op;
    int add;
    while(fgets(lineBuffer, 256, stdin))
    {
        sscanf(lineBuffer,"%c %d", &op, &add);
        addLine(op,add);
    }
}

void initCache(void)
{
    CACHE = (Cache*)malloc(sizeof(Cache));
    CACHE->sets = (Set*)calloc(NUM_SETS,sizeof(Set));
    int i;
    for(i = 0; i < NUM_SETS; i++)
    {
        CACHE->sets[i].blocks = (Block*)calloc(SET_ASSOCIATIVITY,sizeof(Block));
        
        int j;
        for(j = 0; j < SET_ASSOCIATIVITY; j++)
        {
            CACHE->sets[i].blocks[j].address = -1;
            CACHE->sets[i].blocks[j].dirty = 0;
            CACHE->sets[i].blocks[j].lastused = 0;
        }
    }

    CACHE->hits = 0;
    CACHE->misses = 0;
    CACHE->memrefs = 0;
    CACHE->cacheReferences = 0;
}

void deinitCache(void)
{
    int i;
    for(i = 0; i < NUM_SETS; i++)
    {
        free(CACHE->sets[i].blocks);
    }
    free(CACHE->sets);
    free(CACHE);
}

void resetCache(void)
{
    int i;
    for(i = 0; i < NUM_SETS; i++)
    {
        int j;
        for(j = 0; j < SET_ASSOCIATIVITY; j++)
        {
            CACHE->sets[i].blocks[j].address = -1;
            CACHE->sets[i].blocks[j].dirty = 0;
            CACHE->sets[i].blocks[j].lastused = 0;
        }
    }

    CACHE->hits = 0;
    CACHE->misses = 0;
    CACHE->memrefs = 0;
    CACHE->cacheReferences = 0;
}

void calculateAddressBits(void)
{
    OFFSET_BITS = logBase(BLOCK_SIZE,2);
    INDEX_BITS = logBase(NUM_SETS,2);
    TAG_BITS = 32 - OFFSET_BITS - INDEX_BITS;
}

unsigned int getTagBits(unsigned int address)
{
    return address >> (OFFSET_BITS+INDEX_BITS);
}

unsigned int getIndexBits(unsigned int address)
{
    address = address << TAG_BITS;
    return address >> (TAG_BITS + OFFSET_BITS);
}

unsigned int getOffsetBits(unsigned int address)
{
    address = address << (TAG_BITS+INDEX_BITS);
    return address >> (TAG_BITS+INDEX_BITS);
}

int lruReplace(Line* l, int lineNum)
{
    int setTarget = 0;
    int replacementSet = 0;
    int lru = CACHE->sets[getIndexBits(l->address)].blocks[0].lastused;
    while(setTarget < SET_ASSOCIATIVITY)
    {
        if(CACHE->sets[getIndexBits(l->address)].blocks[setTarget].lastused < lru)
        {
            lru = CACHE->sets[getIndexBits(l->address)].blocks[setTarget].lastused;
            replacementSet = setTarget;
        }
        setTarget += 1;
    }
    CACHE->sets[getIndexBits(l->address)].blocks[replacementSet].lastused = lineNum;
    CACHE->sets[getIndexBits(l->address)].blocks[replacementSet].address = l->address;
    if(CACHE->sets[getIndexBits(l->address)].blocks[replacementSet].dirty)
    {
        CACHE->memrefs += 1;
    }
    CACHE->sets[getIndexBits(l->address)].blocks[replacementSet].dirty = 0;
    return replacementSet;
}

void cacheLine(Line* l, int lineNum, char cachingMethod)
{
    CACHE->cacheReferences += 1;
    int hit = -1;
    int freeSpace = -1;

    unsigned int setTarget = 0;
    // Check for Existing Identical Tag to Update
    while(setTarget < SET_ASSOCIATIVITY)
    {
        if(getTagBits(CACHE->sets[getIndexBits(l->address)].blocks[setTarget].address) == getTagBits(l->address))
        {
            hit = setTarget;
            CACHE->hits += 1;
            break;
        }
        setTarget += 1;
    }

    // No Existing Identical Tag, Seek Empty Block
    if(hit<0)
    {
        CACHE->misses += 1;
        setTarget = 0;
        while(setTarget < SET_ASSOCIATIVITY)
        {
            if(CACHE->sets[getIndexBits(l->address)].blocks[setTarget].address < 0)
            {
                freeSpace = setTarget;
                break;
            }
            // Iterate to find a blank point in the cache
            setTarget += 1;
        }
    }

    // ---------- Process ----------
    if(cachingMethod == 'T')
    {
        // Write Through Rules
        if(l->operation == 'R')
        {
            if(hit > -1)
            {
                // Read Hit
                CACHE->sets[getIndexBits(l->address)].blocks[hit].lastused = lineNum;
            }
            else
            {
                // Read Miss
                CACHE->memrefs += 1;

                if(freeSpace > -1)
                {
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].lastused = lineNum;
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].address = l->address;
                }
                else
                {
                    // LRU Replacement
                    lruReplace(l,lineNum);
                }
            }
        }
        else if(l->operation == 'W')
        {
            if(hit > -1)
            {
                // Write Hit
                CACHE->memrefs += 1;
                CACHE->sets[getIndexBits(l->address)].blocks[hit].address = l->address;
                CACHE->sets[getIndexBits(l->address)].blocks[hit].lastused = lineNum;
            }
            else
            {
                // Write Miss
                CACHE->memrefs += 1;
            }
        }
    }
    else if(cachingMethod == 'B')
    {
        // Write Back Rules
        if(l->operation == 'R')
        {
            if(hit > -1)
            {
                // Read Hit
                CACHE->sets[getIndexBits(l->address)].blocks[hit].lastused = lineNum;
            }
            else
            {
                // Read Miss
                CACHE->memrefs += 1;
                if(freeSpace > -1)
                {
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].lastused = lineNum;
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].address = l->address;
                }
                else
                {
                    // LRU Replacement
                    lruReplace(l, lineNum);
                }
            }
        }
        else if(l->operation == 'W')
        {
            if(hit > -1)
            {
                // Write Hit
                CACHE->sets[getIndexBits(l->address)].blocks[hit].lastused = lineNum;
                CACHE->sets[getIndexBits(l->address)].blocks[hit].address = l->address;
                CACHE->sets[getIndexBits(l->address)].blocks[hit].dirty = 1;
            }
            else
            {
                // Write Miss
                CACHE->memrefs += 1;
                if(freeSpace > -1)
                {
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].lastused = lineNum;
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].address = l->address;
                    CACHE->sets[getIndexBits(l->address)].blocks[freeSpace].dirty = 1;
                }
                else
                {
                    // LRU Replacement
                    int replacementSet = lruReplace(l, lineNum);
                    CACHE->sets[getIndexBits(l->address)].blocks[replacementSet].dirty = 1;
                }
            }
        }
    }
}

void simulate(char cachingMethod)
{
    int i;
    for(i = 0; i < LINE_LIST->size; i++)
    {
        if(cachingMethod == 'T')
        {
            cacheLine(&LINE_LIST->lines[i], i+1, cachingMethod);
        }
        else if(cachingMethod == 'B')
        {
            cacheLine(&LINE_LIST->lines[i], i+1, cachingMethod);
        }
    }
}

void printHeader(void)
{
    printf("Block size: %d\nNumber of sets: %d\nAssociativity: %d\n", BLOCK_SIZE, NUM_SETS, SET_ASSOCIATIVITY);
    printf("Number of offset bits: %d\nNumber of index bits: %d\nNumber of tag bits: %d\n", OFFSET_BITS, INDEX_BITS, TAG_BITS);
}

void printCacheReport(char cachingMethod)
{
    char* div = "****************************************\n";
    printf("%s",div);
    if(cachingMethod == 'T')
        printf("Write-through with No Write Allocate\n");
    else if(cachingMethod == 'B')
        printf("Write-back with Write Allocate\n");
    printf("%s",div);

    printf("Total number of references: %d\n", CACHE->cacheReferences);
    printf("Hits: %d\n", CACHE->hits);
    printf("Misses: %d\n", CACHE->misses);
    printf("Memory References: %d\n", CACHE->memrefs);
}

void printCache(void)
{
    int i;
    for(i = 0; i < NUM_SETS; i++)
    {
        printf("Set %d: ", i);
        int j;
        for(j = 0; j < SET_ASSOCIATIVITY; j++)
        {
            printf("%d[%d] ", CACHE->sets[i].blocks[j].address, CACHE->sets[i].blocks[j].lastused);
        }
        printf("\n");
    }
}

void printTranslatedLines(void)
{
    int i;
    for(i = 0; i < LINE_LIST->size; i++)
    {
        printf("%c %d %d %d %d\n", LINE_LIST->lines[i].operation, LINE_LIST->lines[i].address, getTagBits(LINE_LIST->lines[i].address), getIndexBits(LINE_LIST->lines[i].address), getOffsetBits(LINE_LIST->lines[i].address));
    }
}

void printInput(void)
{
    printf("Block Size: %d\nNumber of Sets: %d\nSet Associativity: %d\nLines: %d\n", BLOCK_SIZE, NUM_SETS, SET_ASSOCIATIVITY, LINE_LIST->size);
    printLines();
}

double logBase(int num, int base)
{
    return (log(num)/log(base));
}

/*----------------------------------*
 *                MAIN              *
 *----------------------------------*/
int main()
{
    // Initialize Dynamic Lines
    initLines();

    // Parse Input
    parseInput();

    // Create Cache
    initCache();

    // Calculate & Print Common/Shared Cache Information
    calculateAddressBits();
    printHeader();

    // Execute Write-Through, No-Write-Allocate Caching Patterns
    simulate('T');

    // Print Write-Through, No-Write-Allocate Cache Report
    printCacheReport('T');

    // Reset for Next Strategy
    resetCache();
    
    // Execute Write-Back, Write-Allocate Caching Patterns
    simulate('B');

    // Print Write-Back, Write-Allocate Cache Report
    printCacheReport('B');

    // Deinitialize
    deinitLines();
    deinitCache();
}
