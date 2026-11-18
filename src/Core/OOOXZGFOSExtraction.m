#import "OOOXZGFOSExtraction.h"
#import "OOOXZArchivePreflight.h"
#import "OOOXZPathSafety.h"
#include "unzip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

static int gfosMakeDirectory(const char *path) {
    struct stat st;
    if (mkdir(path,0777)==0) return 1;
    return errno==EEXIST && stat(path,&st)==0 && S_ISDIR(st.st_mode);
}
static int gfosMakeParents(const char *path) {
    char tmp[1024]; size_t i,n=strlen(path);
    if (n==0 || n>=sizeof(tmp)) return 0;
    memcpy(tmp,path,n+1);
    for (i=0;i<n;i++) if (tmp[i]=='/' && i>4) {
        tmp[i]=0;
        if (!gfosMakeDirectory(tmp)) return 0;
        tmp[i]='/';
    }
    return 1;
}
static int gfosRemoveTree(const char *path, unsigned depth) {
    struct stat st;
    if (depth > 32 || lstat(path,&st)!=0) return 0;
    if (!S_ISDIR(st.st_mode)) return unlink(path)==0;
    DIR *dir=opendir(path);
    if (!dir) return 0;
    struct dirent *entry;
    int ok=1;
    while ((entry=readdir(dir))!=NULL) {
        char child[1024];
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) continue;
        if (snprintf(child,sizeof(child),"%s/%s",path,entry->d_name)<0 ||
            strlen(path)+1+strlen(entry->d_name)>=sizeof(child) ||
            !gfosRemoveTree(child,depth+1)) ok=0;
    }
    closedir(dir);
    return ok && rmdir(path)==0;
}
BOOL OOGFOSExtractOXZ(NSString *archive, NSString *destination)
{
    BOOL ok=NO;
    NSString *staging=[destination stringByAppendingString:@".partial"];
    const char *archivePath=[archive UTF8String];
    const char *stagePath=[staging UTF8String];
    const char *finalPath=[destination UTF8String];
    unzFile zip=NULL;
    if (!archivePath || !stagePath || !finalPath ||
        !OOGFOSPreflightOXZ(archivePath,NULL) ||
        access(stagePath,F_OK)==0 || access(finalPath,F_OK)==0 ||
        !gfosMakeParents(stagePath) || !gfosMakeDirectory(stagePath)) return NO;
    zip=unzOpen64(archivePath);
    if (!zip) goto finish;
    int rc=unzGoToFirstFile(zip);
    if (rc!=UNZ_OK) goto finish;
    do {
        unz_file_info64 info={0}; char entry[512]={0}; char path[1024];
        if (unzGetCurrentFileInfo64(zip,&info,entry,sizeof(entry),NULL,0,NULL,0)!=UNZ_OK ||
            !OOGFOSSafeOXZEntry(entry,(size_t)info.size_filename)) goto finish;
        entry[info.size_filename]=0;
        if (snprintf(path,sizeof(path),"%s/%s",stagePath,entry)<0 ||
            strlen(stagePath)+1+strlen(entry)>=sizeof(path)) goto finish;
        if (!gfosMakeParents(path)) goto finish;
        size_t nlen=strlen(entry);
        if (nlen && entry[nlen-1]=='/') {
            if (!gfosMakeDirectory(path)) goto finish;
        } else {
            FILE *out=NULL; unsigned char buffer[16384];
            unsigned long long seen=0; int n=0,failed=0;
            if (unzOpenCurrentFile(zip)!=UNZ_OK) goto finish;
            out=fopen(path,"wb");
            if (!out) failed=1;
            while (!failed && (n=unzReadCurrentFile(zip,buffer,sizeof(buffer)))>0) {
                seen+=(unsigned)n;
                if (seen>info.uncompressed_size || fwrite(buffer,1,(size_t)n,out)!=(size_t)n) failed=1;
            }
            if (n<0 || seen!=info.uncompressed_size) failed=1;
            if (out && fclose(out)!=0) failed=1;
            if (unzCloseCurrentFile(zip)!=UNZ_OK) failed=1;
            if (failed) goto finish;
        }
        rc=unzGoToNextFile(zip);
    } while (rc==UNZ_OK);
    if (rc!=UNZ_END_OF_LIST_OF_FILE) goto finish;
    if (unzClose(zip)!=UNZ_OK) {zip=NULL;goto finish;}
    zip=NULL;
    if (rename(stagePath,finalPath)!=0) goto finish;
    ok=YES;
finish:
    if (zip) unzClose(zip);
    if (!ok) gfosRemoveTree(stagePath,0);
    return ok;
}
