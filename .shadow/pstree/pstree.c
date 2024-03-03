#include <stdio.h>
#include <assert.h>
#include <getopt.h>
int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  const struct option table[] = {
      {"show-pids"   , no_argument, NULL, 'p'},
      {"numeric-sort", no_argument, NULL, 'n'},
      {"version"     , no_argument, NULL, 'V'},
  };
  int opt = 0;
  while((opt = getopt_long(argc, argv, "pnV", table, NULL))){
      switch (opt){
          case 'p':
              {
                  printf("p\n");
              }
          case 'n':
              {
                  printf("n\n");
              }
          case 'V':
              {
                  printf("V\n");
              }

      }
  }
  return 0;
}
