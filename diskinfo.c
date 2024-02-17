// Name: Omar Madhani

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "diskfunctions.h"

// Sources: Some of the code used in my program was provided by the course instructor Ahmad Abdullah.

boot_t boot_sector;
char *fat_table;
int fat_size = 0;

int main(int argc, char *argv[]) {
    
    // Error handling
    if (argc >= 3) {
        printf("Too many parameters. Please only provide a disk filename\n");
        return -1;
    }
    
    if (argc < 2) {
        printf("Not enough arguments. Please provide a disk filename\n");
        return -1;
    }

    FILE *fileSystemPtr = fopen(argv[1], "rb");
    if (fileSystemPtr == NULL) {
        printf("Disk file/image \"%s\" not found\n", argv[1]);
        return -1;
    }
    
    fread(&boot_sector, sizeof(boot_sector), 1, fileSystemPtr);
    mainHelperFunction(fileSystemPtr, argv);
    fclose(fileSystemPtr);
    return 0;
}

// ensures that the main function is concise
void mainHelperFunction(FILE *fileSystemPtr, char *argv[]) {
    
    // code provided by instructor
    // ===============================================================
    int free_bytes;
    int fat_mem_size;
    
    /* Allocate space for memory copy of the FAT */
    fat_size = boot_sector.total_sectors - 33 + 2;
    fat_mem_size = (fat_size & 0x01) ? (fat_size*3+1)/2 : fat_size*3/2;
    fat_table = malloc(fat_mem_size);
   
    /* read a FAT copy */
    fseek(fileSystemPtr, 0x200, SEEK_SET);
    fread(fat_table, fat_mem_size, 1, fileSystemPtr);

    // ===============================================================

    getDiskLabel(fileSystemPtr);
    long int totalDiskSize = boot_sector.total_sectors*boot_sector.bytes_per_sector;
    
    int fileCount = 0;
    countNumberOfFiles(fileSystemPtr, (boot_sector.reserved_sectors + boot_sector.fats * boot_sector.sectors_per_fat) * 512, &fileCount);

    printf("OS Name: %s\n", boot_sector.name);
    printf("Label of the disk: %s\n", boot_sector.label);
    printf("Total size of the disk: %ld\n", totalDiskSize);
    printf("Free size of the disk: %d\n", getFreeBlocks() * boot_sector.bytes_per_sector);
    printf("The number of files in the disk: %d\n", fileCount);
    printf("Number of FAT copies: %d\n", boot_sector.fats);
    printf("Sectors per FAT: %d\n", boot_sector.sectors_per_fat);
}

// This function was provided by the instructor. It returns the FAT value at a given index
int get_fat(int i) {
   int j;
   
   if (fat_table == NULL || fat_size == 0) {
      /* File system data hasn't loaded yet */
      return -1;
   }
   if (i & 0x01) {
      j = (1 + i * 3)/2;
      return ((fat_table[j-1] & 0xF0) >> 4) + (fat_table[j] << 4);
   } else {
      j = i * 3 / 2;
      return ((fat_table[j+1] & 0x0F) << 8) + fat_table[j];
   }
}

// This function was provided by the instructor. It returns the number of unused sectors
int getFreeBlocks() {
   int i, free_bytes = 0;
   
   if (fat_table == NULL || fat_size == 0) {
      /* File system data hasn't loaded yet */
      return -1;
   }
   
   for (i=2; i< fat_size; i++) {
      if (get_fat(i) == 0) {
         free_bytes += 1;
      }
   }
   return free_bytes;
}

// null terminates a filename or file extension
void addNullTerminatorToString(char* fileNameOrExtension) {
    int index = 0;
    while (index < strlen(fileNameOrExtension) && fileNameOrExtension[index] != ' ') {
        index++;
    }
    fileNameOrExtension[index] = '\0';
}

// obtains the label of the disk
void getDiskLabel(FILE* fileSystemPtr) {
    entry_t rootEntry;
    
    // position 19*512
    int startOfRootDirectory = (boot_sector.reserved_sectors + boot_sector.fats * boot_sector.sectors_per_fat) * 512;
    fseek(fileSystemPtr, startOfRootDirectory, SEEK_SET);
    for (int index = 0; index < boot_sector.root_entries; index++) {
        fread(&rootEntry, sizeof(rootEntry), 1, fileSystemPtr);
        
        // if we read an attribute byte of 0x08, we know that we have found the disk label
        if (rootEntry.attributes == 0x08) {
            strcpy(boot_sector.label, rootEntry.filename);
            addNullTerminatorToString(boot_sector.label);
            return;
        }
    }
    // if we could not find an attribute 0x08 in the root directory
    strcpy(boot_sector.label, "NO NAME");
}

// counts the number of files in the disk
void countNumberOfFiles(FILE* fileSystemPtr, int offset, int* fileCount) {
    entry_t directoryEntry;

    // an array to store the logical clusters of subdirectories 
    int subdirectoryLogicalClusters[boot_sector.root_entries];

    int index;

    // initialize each element of the array to 0
    for (index = 0; index < boot_sector.root_entries; index++) {
        subdirectoryLogicalClusters[index] = 0;
    }
    
    fseek(fileSystemPtr, offset, SEEK_SET);

    for (index = 0; index < boot_sector.root_entries; index++) {
        fread(&directoryEntry, sizeof(directoryEntry), 1, fileSystemPtr);
        if (directoryEntry.filename[0] == 0) {
            break;
        }
        if (directoryEntry.filename[0] == 0xE5) {
            continue;
        }

        // we do not consider long filenames or the disk label to be a file
        if (directoryEntry.attributes == 0x0F || directoryEntry.attributes == 0x08) {
            continue;
        }

        // if we encounter a subdirectory, add its first logical cluster to the array
        if ((directoryEntry.attributes & 0x10) != 0) {
            subdirectoryLogicalClusters[index] = directoryEntry.cluster;
        } else {
            *fileCount += 1;
        }
    }
    
    // iterate through our subdirectory array and recursively call the function on each subdirectory
    int subdirectoryIndex = 0;
    while (subdirectoryIndex < boot_sector.root_entries) {
        if (subdirectoryLogicalClusters[subdirectoryIndex] != 0) {

            // we add 64 to the physical cluster to skip "." and ".." entries
            countNumberOfFiles(fileSystemPtr, (subdirectoryLogicalClusters[subdirectoryIndex]+31)*512+32+32, fileCount);
        }
        subdirectoryIndex++;
    }

}
