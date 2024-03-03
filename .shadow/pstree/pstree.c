#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
bool show_pids_flag = false, nemuric_sort_flag = false, version_flag = false;
//int f[100005];

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

typedef struct TreeNode{
    int pid;
    char name[25];
    struct TreeNode* leftchild;
    struct TreeNode* rightsibling;
}TreeNode;

TreeNode* CreateTreeNode(int pid, const char* name){
    TreeNode *newnode = (TreeNode*)malloc(sizeof(TreeNode));
    if(newnode == NULL) return NULL;
    newnode->pid = pid;
    strcpy(newnode->name, name);
    newnode->leftchild = NULL;
    newnode->rightsibling = NULL;
    return newnode;
}

TreeNode* pNode[100005];
int cnt;
int parent_son[10005][2];

void visit_procfs(){
    DIR *directory;
    struct dirent *entry;

    directory = opendir("/proc");
    if(directory == NULL){
        perror("opendir");
        exit(1);
    }
    while((entry = readdir(directory)) != NULL){
        char *pid_dir = entry->d_name;
        if(isNumber(pid_dir)){
            //printf("proc/%s\n", pid_dir);
            char filename[40] = "";
            int pid = atoi(pid_dir);
            sprintf(filename, "/proc/%d/stat", pid);
            FILE* fp = fopen(filename, "r");
            if(fp == NULL){
                perror("fopen");
                continue;
            }
            int ppid = 0;
            char pid_name[20];
            char Umask[5];
            fscanf(fp, "%d %s %s %d", &pid, pid_name, Umask, &ppid);
            if(pid > 100005 || ppid > 100005){
                fprintf(stderr,"The pNode array is too small.");
                assert(0);
            }
            pNode[pid] = CreateTreeNode(pid, pid_name);
            parent_son[cnt][0] = ppid;
            parent_son[cnt][1] = pid;
            cnt++;
            //printf("pid = %d name = %s ppid = %d\n", pid, pid_name, ppid);
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



  

  return 0;
}
