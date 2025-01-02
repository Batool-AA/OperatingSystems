#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

int main() {
    struct statvfs disk_info;
    statvfs("/", &disk_info);

    // 1. Read details about hard disk
    unsigned long long total_size = disk_info.f_blocks * disk_info.f_frsize;
    unsigned long long free_space = disk_info.f_bfree * disk_info.f_frsize;
    
    printf("Total disk size: %lld bytes\n", total_size);
    printf("Free space available: %.lld bytes\n", free_space);

    // 2. Take input - sentence + name of file
    char sentence[256];
    char file_name[256]; 

    printf("Enter a sentence: ");
    scanf("%s", sentence);

    printf("Enter the name of the file (e.g., file.txt): ");
    scanf("%s", file_name);

    // 3. Create the file with several instances of the sentence
    free_space = free_space / 1024;
    unsigned long long instances = free_space / 100000;

    FILE *file = fopen(file_name, "w");
    for (unsigned long long i = 0; i < instances; i++) {
        fputs(sentence, file);
    }
    fclose(file);

    printf("File '%s' created with %llu instances of the sentence.\n", file_name, instances);
    return 0;
}


