#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

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

bool isNumber(const char *str){
    int len = strlen(str);
    const char* digit = str;
    for (int i = 0; i < len; i++){
        if(!isdigit(*digit)){
            return false;
        }
        digit++;
    }
    return true;
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
  struct dirent *entry;

  directory = opendir("/proc");
  if(directory == NULL){
      perror("opendir");
      return 1;
  }
  while((entry = readdir(directory)) != NULL){
      char *d_name = entry->d_name;
      if(isNumber(d_name)){
          printf("proc/%s", d_name);
      }
  }

  

  return 0;
}
