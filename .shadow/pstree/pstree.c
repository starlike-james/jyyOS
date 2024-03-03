#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>


bool show_pids_flag = false, nemuric_sort_flag = false, version_flag = false;

void parse_arg(int argc, char *argv[]){
   const struct option table[] = {
      {"show-pids"   , no_argument, NULL, 'p'},
      {"numeric-sort", no_argument, NULL, 'n'},
      {"version"     , no_argument, NULL, 'V'},
      {0             , 0          , NULL,  0 },
  };
  int opt = 0;
  while((opt = getopt_long(argc, argv, "pnV", table, NULL)) != -1){
      switch (opt){
          case 'p': show_pids_flag = true; break;
          case 'n': nemuric_sort_flag = true; break;
          case 'V': version_flag = true; break;
          default: printf("default: invalid argument\n"); break;
      }
  }

}
int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  parse_arg(argc, argv);
 
  if(version_flag == true){
      fprintf(stderr, "wzy's pstree\n");
      return 0;
  }

  DIR *directory;
  

  

  return 0;
}
