/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
//#include "android_filesystem_config.h"

// This program takes a list of files and directories (indicated by a
// trailing slash) on the stdin, and prints to stdout each input
// filename along with its desired uid, gid, and mode (in octal).
// The leading slash should be stripped from the input.
//
// Example input:
//
//    system/etc/dbus.conf
//    data/app/
//
// Output:
//
//    system/etc/dbus.conf 1002 1002 440
//    data/app 1000 1000 771
//
// Note that the output will omit the trailing slash from
// directories.
struct fs_path_config {
    unsigned mode;
    unsigned uid;
    unsigned gid;
    const char *prefix;
};
struct android_id_info {
    const char *name;
    unsigned aid;
};
static char *rule_configs = "fs_config.h";

static struct fs_path_config *android_dirs = NULL;
static struct fs_path_config *android_files = NULL;

static inline int estimate_dirs_or_files(char *path)
{
	int i;
	int is_dir;
	char buffer[1024];

	if(path)
	{
		strncpy(buffer, path, sizeof(buffer));
		for(i = 0; i < 1024 && buffer[i]; ++i)
		{
			switch(buffer[i]){
				case '\n':
					i = 1025;
					break;
				case '/':
					is_dir = 1;
					break;
				default:
					is_dir = 0;
					break;
			}
		}
	}

	return is_dir;
}

static void read_user_rules()
{
	int alloc_files = 1024;
	int alloc_dirs  = 100;
	
	char buff[1024] = {0};
	
	char current_absolute_path[1024];
	//获取当前程序绝对路径
	int cnt = readlink("/proc/self/exe", current_absolute_path, 1024);
	if (cnt < 0 || cnt >= 1024)
	{
	    //printf("Error :*********\n");
		return ;
	}
	//获取当前目录绝对路径，即去掉程序名
	int i;
	for (i = cnt; i >=0; --i)
	{
		if (current_absolute_path[i] == '/')
		{
			memcpy(&current_absolute_path[i+1],rule_configs,strlen(rule_configs));
			current_absolute_path[i+1+strlen(rule_configs)] = '\0';
			break;
		
		}
	}
	
	//printf("current path %s\n",current_absolute_path);
	FILE * f = fopen(current_absolute_path,"r");
	if(!f)
	{
		//printf("WARNING : Not Found %s.\n",RULES_FILE);
		return ;
	}

	android_dirs = malloc(alloc_dirs * sizeof(struct fs_path_config));
	if(!android_dirs)
	{
		//printf("ERROR : Not Enough Memory Space.\n");
		return ;
	}
	memset(android_dirs, 0, alloc_dirs * sizeof(struct fs_path_config));
	
	android_files = malloc(alloc_files * sizeof(struct fs_path_config));
	if(!android_files)
	{
		//printf("ERROR : Not Enough Memory Space.\n");
		free(android_dirs);
		return ;
	}
	memset(android_files, 0, alloc_dirs * sizeof(struct fs_path_config));

	//int i;
	int is_dir;
	
	struct fs_path_config *android_dirs_org = android_dirs;
	struct fs_path_config *android_files_org = android_files;

	while(fgets(buff, sizeof(buff)-1, f))
	{
		if(strlen(buff) <= 10)
		{
		//	printf("Warnning : unvalid para in fs_config.h\n");
			continue;
		
		}
		for(i = 0; buff[i] && isspace(buff[i]); ++i)
			if(buff[i] == '\0' ||  buff[i] == '#') continue;
		
		char *origin = strdup(buff);
		char *pmode = strtok(buff + i, "\t\n");
		unsigned mode = strtoul(pmode, NULL, 0);
		unsigned uid = atoi(strtok(NULL,"\t\n"));
		unsigned gid = atoi(strtok(NULL,"\t\n"));
		char *prefix = strtok(NULL,"\t\n");
	 //printf("mode = %u uid = %u gid = %u prefix = %s \n",mode,uid,gid,prefix);	
		
		is_dir = estimate_dirs_or_files(prefix);	
		if(is_dir)
		{
			android_dirs_org->mode = mode;
			android_dirs_org->uid = uid;
			android_dirs_org->gid = gid;
			android_dirs_org->prefix = strdup(prefix);
			android_dirs_org++;
		}else{
			android_files_org->mode = mode;
			android_files_org->uid = uid;
			android_files_org->gid = gid;
			android_files_org->prefix = strdup(prefix);
			android_files_org++;
		}
		memset(buff,0,sizeof(buff));
	}
		
}


static inline void fs_config(const char *path, int dir,
                             unsigned *uid, unsigned *gid, unsigned *mode)
{
    struct fs_path_config *pc;
    int plen;

    pc = dir ? android_dirs : android_files;
    if(!pc)
	{
		return ; 
	}
	plen = strlen(path);

    for(; pc->prefix; pc++){
        int len = strlen(pc->prefix);
        if (dir) {
            if(plen < len) continue;
            if(!strncmp(pc->prefix, path, len)) break;
            continue;
        }
        /* If name ends in * then allow partial matches. */
        if (pc->prefix[len -1] == '*') {
            if(!strncmp(pc->prefix, path, len - 1)) break;
        } else if (plen == len){
            if(!strncmp(pc->prefix, path, len)) break;
        }
    }
    *uid = pc->uid;
    *gid = pc->gid;
	*mode = (*mode & (~07777)) | pc->mode;
#if 0
    fprintf(stderr,"< '%s' '%s' %d %d %o >\n",
            path, pc->prefix ? pc->prefix : "", *uid, *gid, *mode);
#endif
}


int main(int argc, char** argv) {
  char buffer[1024] ={0};

  read_user_rules();

  while (fgets(buffer, 1023, stdin) != NULL) {
    int is_dir = 0;
    int i;
    for (i = 0; i < 1024 && buffer[i]; ++i) {
      switch (buffer[i]) {
        case '\n':
          buffer[i-is_dir] = '\0';
          i = 1025;
          break;
        case '/':
          is_dir = 1;
          break;
        default:
          is_dir = 0;
          break;
      }
    }

    unsigned uid = 0, gid = 0, mode = 0;
    fs_config(buffer, is_dir, &uid, &gid, &mode);
    printf("%s %d %d %o\n", buffer, uid, gid, mode);
  }
  return 0;
}
