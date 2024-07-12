#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "fat32.h"

struct fat32hdr *hdr;

u8* ClustersMark;

u32 DataSecCnt;
u32 ClusterCnt;
u32 DataSec;

void *mmap_disk(const char* fname){
    int fd = open(fname, O_RDWR);
    size_t size = lseek(fd, 0, SEEK_END);

    struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if(hdr == MAP_FAILED){
        perror("mmap");
        close(fd);
        exit(1);
    }

    close(fd);

    assert(hdr->Signature_word == 0xaa55);
    assert(hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec == size);
    
    return hdr;
}

void *sec_to_addr(u32 n){
    return ((u8 *)hdr) + n * hdr->BPB_BytsPerSec;
}

u32 cluster_to_sec(u32 n){
    return DataSec + (n - 2) * hdr->BPB_SecPerClus; 
}

void *cluster_to_addr(u32 n){
    u32 sec = cluster_to_sec(n);
    return sec_to_addr(sec);
}

void traverse_clusters();

void traverse_dir(u32 clusId){
    int ndents = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus / sizeof(struct fat32dent);

    for(int d = 0; d < ndents; d++){
        struct fat32dent *dent = (struct fat32dent *)cluster_to_addr(clusId) + d;
        if(dent->DIR_Attr == ATTR_LONG_NAME){

        }else{

        }
    }
}


int main(int argc, char *argv[]) {

    assert(sizeof(struct fat32lfndent) == 32);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
        exit(1);
    }

    hdr = mmap_disk(argv[1]);

    DataSecCnt = hdr->BPB_TotSec32 - hdr->BPB_RsvdSecCnt - hdr->BPB_NumFATs * hdr->BPB_FATSz32;
    ClusterCnt = DataSecCnt / hdr->BPB_SecPerClus;
    DataSec = hdr->BPB_RsvdSecCnt + hdr->BPB_FATSz32 * hdr->BPB_NumFATs;

    ClustersMark = malloc(ClusterCnt);

    traverse_clusters();

    for(int i = 0; i < ClusterCnt; i++){
        if(ClustersMark[i] == DIRT){
            u32 clusId = i + 2;
            traverse_dir(clusId);
        }
    }

    munmap(hdr, hdr->BPB_BytsPerSec * hdr->BPB_TotSec32);
    return 0;
}


bool check_dir(void *ClusterAddr){
    int ndents = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus / sizeof(struct fat32dent);
    int bmpcnt = 0;
    for(int d = 0; d < ndents; d++){
        struct fat32dent *dent = (struct fat32dent *)ClusterAddr + d;
        if(dent->DIR_Name[0] == 0x00 || dent->DIR_Name[0] == 0xe5 || dent->DIR_Attr & ATTR_HIDDEN){
            continue;
        }
        if(dent->DIR_Name[8] == 'B' && dent->DIR_Name[9] == 'M' && dent->DIR_Name[10] == 'P'){
            bmpcnt++;
        }
        if(bmpcnt > 10){
            return true;
        }
    }
    return false;
}

bool check_bmpheader(void *ClusterAddr){
    struct bmpheader* bhr = ClusterAddr;
    if(bhr->magic == 0x4d42){
        return true;
    }else{
        return false;
    }
}

bool check_used(void *ClusterAddr){
    u32 *point = ClusterAddr;
    int nword = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus / sizeof(u32);
    for(int i = 0; i < nword; i++){
        if(*point != 0){
            return true;
        }
        point++;
    }
    return false;
}


void traverse_clusters(){
    for(u32 i = 0; i < ClusterCnt; i++){
        u32 clusId = i + 2;
        u8 *addr = cluster_to_addr(clusId);
        // printf("%u ", clusId);
        if(check_bmpheader(addr)){
            ClustersMark[i] = BMPHEADER;
            // printf("BMPHEADER\n");
        }else if(check_dir(addr)){
            ClustersMark[i] = DIRT;
            // printf("DIRT\n");
        }else if(check_used(addr)){
            ClustersMark[i] = BMPDATA;
            // printf("BMPDATA\n");
        }else{
            ClustersMark[i] = UNUSED;
            // printf("UNUSED\n");
        }
    }
}
