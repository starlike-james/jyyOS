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
    char name[64];
    struct TreeNode* leftchild;
    struct TreeNode* rightsibling;
}TreeNode;

TreeNode* CreateTreeNode(int pid, const char* name){
    TreeNode *newnode = (TreeNode*)malloc(sizeof(TreeNode));
    if(newnode == NULL) return NULL;
    newnode->pid = pid;
    if(strlen(name) >= 64){
        assert(0);
    }
    strcpy(newnode->name, name);
    newnode->leftchild = NULL;
    newnode->rightsibling = NULL;
    return newnode;
}

TreeNode* pNode[200005];
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
            if(pid > 200005 || ppid > 200005){
                fprintf(stderr,"The pNode array is too small.");
                assert(0);
            }

            /*if(pNode[pid] != NULL){
                pNode[pid]->name =
            }*/
            pNode[pid] = CreateTreeNode(pid, pid_name);
            parent_son[cnt][0] = ppid;
            parent_son[cnt][1] = pid;
            cnt++;
            //printf("pid = %d name = %s ppid = %d\n", pid, pid_name, ppid);
        }
    }
    pNode[0] = CreateTreeNode(0, "root");
}

void buildtree(){
  for(int i = 0; i < cnt; i++){
      int ppid = parent_son[i][0];
      int pid = parent_son[i][1];
      
      TreeNode *head = pNode[ppid]->leftchild;
      const char *pid_name = pNode[pid]->name;
      if(nemuric_sort_flag){
          if(head == NULL || head->pid > pid){
              pNode[pid]->rightsibling = head;
              pNode[ppid]->leftchild = pNode[pid];
              continue;
          }
          TreeNode *current = head;
          while(current->rightsibling != NULL && current->rightsibling->pid < pid){
              current = current->rightsibling; 
          }
          pNode[pid]->rightsibling = current->rightsibling;
          current->rightsibling = pNode[pid];
      }
      else
      {
          if(head == NULL || (strcmp(head->name, pid_name) > 0) || (strcmp(head->name, pid_name) == 0 && head->pid > pid)){
              pNode[pid]->rightsibling = head;
              pNode[ppid]->leftchild = pNode[pid];
              continue;
          }
          TreeNode *current = head;
          const char *sibling_name = current->rightsibling->name;
          int sibling_pid = current->rightsibling->pid;
          while(current->rightsibling != NULL && (strcmp(sibling_name, pid_name) > 0 || (strcmp(sibling_name, pid_name) == 0 && sibling_pid < pid)) ){
              current = current->rightsibling;
              sibling_name = current->rightsibling->name;
              sibling_pid = current->rightsibling->pid;
          }
          pNode[pid]->rightsibling = current->rightsibling;
          current->rightsibling = pNode[pid];
      }

  }
}

int dep;
void traversal(TreeNode *current){
    if(current == NULL){
        return;
    }
    for(int i = 0; i < dep; i++){
        printf("\t");
    }
    printf("%s", current->name);
    if(show_pids_flag){
        printf("(%d)\n", current->pid);
    }
    else{
        printf("\n");
    }
    dep++;
    for (TreeNode *s = current->leftchild;
            s != NULL; s = s->rightsibling) {
        traversal(s);
    }
    dep--;
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
    visit_procfs();
    buildtree();
    traversal(pNode[1]);
    return 0;
}
