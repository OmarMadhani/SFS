// Name: Omar Madhani
// V00978484

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
    
    printf("\nROOT\n==================\n");
    printFilesAndSubdirectories(fileSystemPtr, (boot_sector.reserved_sectors + boot_sector.fats * boot_sector.sectors_per_fat) * 512);
    printf("\n");
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

// null terminates a filename or file extension
void addNullTerminatorToString(char* fileNameOrExtension) {
    int index = 0;
    while (index < strlen(fileNameOrExtension) && fileNameOrExtension[index] != ' ') {
        index++;
    }
    fileNameOrExtension[index] = '\0';
}

// prints up to 10 additional spaces on a line depending on the file size
void printUpToTenSpaces(int fileSize) {
    int strIndex = 0;
    char size[10];
    sprintf(size, "%d", fileSize);
    while (strIndex < strlen(size) && size[strIndex] != ' ') {
        strIndex++;
    }
    int index;
    for (index = 0; index < 10 - strIndex; index++) {
        printf(" ");
    }
}

// prints up to 20 additional spaces on a line depending on the filenam
void printUpToTwentySpaces(char* fileName, char* fileExtension) {
    int strIndex = 0;
    char fullFileName[20];
    strcpy(fullFileName, "");
    strcat(fullFileName, fileName);
    strcat(fullFileName, ".");
    strcat(fullFileName, fileExtension);
    while (strIndex < strlen(fullFileName) && fullFileName[strIndex] != ' ') {
        strIndex++;
    }
    int index;
    for (index = 0; index < 20 - strIndex; index++) {
        printf(" ");
    }
}

// prints the attributes of a file, including its size and creation date/time
void printFileName(entry_t directoryEntry) {
    char fileName[9];
    strncpy(fileName, directoryEntry.filename,8);
    addNullTerminatorToString(fileName);
    char fileExtension[4];
    strncpy(fileExtension, directoryEntry.extension,3);
    addNullTerminatorToString(fileExtension);
             
    printf("F %d", directoryEntry.size);
    printUpToTenSpaces(directoryEntry.size);
    printf(" ");
    printf("%s.%s", fileName, fileExtension);
    printUpToTwentySpaces(fileName, fileExtension);
    
    // Month the file was created 
    printf(" %.2d/", ((directoryEntry.create_date & 0x01E0)) >> 5);
    // Day the file was created
    printf("%.2d/", (directoryEntry.create_date & 0x1F));
    // Year the file was created
    printf("%d", ((directoryEntry.create_date >> 9) & 0x7F) + 1980);

    // Hour the file was created
    printf(" %.2d:", ((directoryEntry.create_time >> 11) & 0x1f));
    // Minute the file was created
    printf("%.2d:",((directoryEntry.create_time >> 5) & 0x3f));
    // Second the file was created
    printf("%.2d", (directoryEntry.create_time & 0x1F) * 2);
    printf("\n");
}

void printFilesAndSubdirectories(FILE* fileSystemPtr, int offset) {
    entry_t directoryEntry;
    
    // an array to store the logical clusters of subdirectories 
    int subdirectoryLogicalClusters[boot_sector.root_entries];
    char subdirectoryNames[boot_sector.root_entries][9];

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
        
        if ((directoryEntry.attributes & 0x10) != 0) {
            
            // if we encounter a subdirectory, add its first logical cluster to the array
            subdirectoryLogicalClusters[index] = directoryEntry.cluster;
            char dirName[9];
            strncpy(dirName, directoryEntry.filename,8);
            addNullTerminatorToString(dirName);
            strcpy(subdirectoryNames[index], dirName);
            printf("D            %s\n", dirName);
            
        } else {
            printFileName(directoryEntry);
        }
        
    }
    
    // iterate through our subdirectory array and recursively call the function on each subdirectory
    int subdirectoryIndex = 0;
    while (subdirectoryIndex < boot_sector.root_entries) {
        if (subdirectoryLogicalClusters[subdirectoryIndex] != 0) {
            printf("\n%s\n==================\n", subdirectoryNames[subdirectoryIndex]);
            
            // we add 64 to the physical cluster to skip "." and ".." entries
            printFilesAndSubdirectories(fileSystemPtr, (subdirectoryLogicalClusters[subdirectoryIndex]+31)*512+32+32);
        }
    subdirectoryIndex++;
    }

}
