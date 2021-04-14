#define _GNU_SOURCE
#include "lazycopy.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
// +------------------+--------------------------------------
// | Declarations     |
// +------------------+

void segfault_handler(int, siginfo_t*, void*);
bool if_writting = false;
void* chunk_start = NULL;
void* new_chunk_start = NULL;
void* chunk_end = NULL;
void* new_chunk_end = NULL;

// +------------------+--------------------------------------
// | Functions        |
// +------------------+

/**
 * This function will be called at startup so you can set up a signal handler.
 */
void chunk_startup() {
  // Make a sigaction struct to hold our signal handler information
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = segfault_handler;
  sa.sa_flags = SA_SIGINFO;

  // Set the signal handler, checking for errors
  if (sigaction(SIGSEGV, &sa, NULL) != 0) {
    perror("sigaction failed");
    exit(2);
  }
}

/**
 * This function should return a new chunk of memory for use.
 *
 * \returns a pointer to the beginning of a 64KB chunk of memory that can be read, written, and
 * copied
 */
void* chunk_alloc() {
  // Call mmap to request a new chunk of memory. See comments below for description of arguments.
  void* result = mmap(NULL, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  // Arguments:
  //   NULL: this is the address we'd like to map at. By passing null, we're asking the OS to
  //   decide. CHUNKSIZE: This is the size of the new mapping in bytes. PROT_READ | PROT_WRITE: This
  //   makes the new reading readable and writable MAP_ANONYMOUS | MAP_SHARED: This makes a new
  //   mapping to cleared memory instead of a file,
  //                               which is another use for mmap. MAP_SHARED makes it possible for
  //                               us to create shared mappings to the same memory.
  //   -1: We're not connecting this memory to a file, so we pass -1 here.
  //   0: This doesn't matter. It would be the offset into a file, but we aren't using one.

  // Check for an error
  if (result == MAP_FAILED) {
    perror("mmap failed in chunk_alloc");
    exit(2);
  }

  // Everything is okay. Return the pointer.
  return result;
}

/**
 * Create a copy of a chunk by copying values eagerly.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_eager(void* chunk) {
  // First, we'll allocate a new chunk to copy to
  void* new_chunk = chunk_alloc();

  // Now copy the data
  memcpy(new_chunk, chunk, CHUNKSIZE);

  // Return the new chunk
  return new_chunk;
}

/**
 * Create a copy of a chunk by copying values lazily.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_lazy(void* chunk) {
  // Just to make sure your code works, this implementation currently calls the eager copy version
  // return chunk_copy_eager(chunk);

  // Your implementation should do the following:
  // 1. Use mremap to create a duplicate mapping of the chunk passed in
  void* new_chunk = mremap(chunk, 0, CHUNKSIZE, MREMAP_MAYMOVE, NULL);
  if (new_chunk == MAP_FAILED) {
    perror("mmap failed in chunk_copy_lazy");
    exit(2);
  }
  memcpy(new_chunk, chunk, CHUNKSIZE);

  // 2. Mark both mappings as read-only
  mprotect(chunk, CHUNKSIZE, PROT_READ);
  mprotect(new_chunk, CHUNKSIZE, PROT_READ);

  // 3. Keep some record of both lazy copies so you can make them writable later.
  //    At a minimum, you'll need to know where the chunk begins and ends.
  chunk_start = chunk;
  new_chunk_start = new_chunk;
  chunk_end = (chunk + CHUNKSIZE);
  new_chunk_end = (new_chunk + CHUNKSIZE);

  // Later, if either copy is written to you will need to:
  // 1. Save the contents of the chunk elsewhere (a local array works well)
  if (if_writting == true) {
  }
  // 2. Use mmap to make a writable mapping at the location of the chunk that was written
  // 3. Restore the contents of the chunk to the new writable mapping

  return new_chunk;
}

// +------------------+--------------------------------------
// | Segfault Handler |
// +------------------+

void segfault_handler(int signal, siginfo_t* info, void* ctx) {
  printf("SEGFAULT!!!!!\n");
  if (((intptr_t)info->si_addr < (intptr_t)chunk_end &&
       (intptr_t)info->si_addr > (intptr_t)chunk_start) ||
      ((intptr_t)info->si_addr < (intptr_t)new_chunk_end &&
       (intptr_t)info->si_addr > (intptr_t)new_chunk_start)) {
    if_writting = true;
    printf("if_writting: %d\n", if_writting);
  }

  // exit code
  exit(1);
}
