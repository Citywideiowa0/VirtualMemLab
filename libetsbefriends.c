#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// TODO: Implement your signal handling code here!
void segfault_handler(int signal, siginfo_t* info, void* ctx);

__attribute__((constructor)) void init() {
  printf("This code runs at program startup\n");

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

void segfault_handler(int signal, siginfo_t* info, void* ctx) {
  // initialize helpful messages
  char* messages[5];
  messages[0] = "Every programmer has this issue, you're not alone!\n";
  messages[1] = "Hang in there, you got this!\n";
  messages[2] = "You go girl!\n";
  messages[3] = "You got a segfault, but thats it! Keep trying!!!\n";
  messages[4] = "Yo got this!\n";

  // initialize randum number generator
  time_t t;
  srand((unsigned)time(&t));

  // print helpful message
  printf("%s", messages[rand() % 5]);

  // exit code
  exit(1);
}
