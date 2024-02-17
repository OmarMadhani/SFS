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
    if (argc >= 4) {
        printf("Too many parameters. Please only provide a disk filename and a file to copy\n");
        return -1;
    }

    if (argc < 3) {
        printf("Not enough arguments. Please provide a disk filename and a file to copy\n");
        return -1;
    }

    FILE *fileSystemPtr = fopen(argv[1], "rb");
    if (fileSystemPtr == NULL) {
        printf("Disk file/image \"%s\" not found\n", argv[1]);
        return -1;
    }
    
    fread(&boot_sector, sizeof(boot_sector), 1, fileSystemPtr);
    mainHelperFunction(fileSystemPtr, argv);
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

    diskget(fileSystemPtr, argv[2]);
    fclose(fileSystemPtr);
}

// modified version of the get_fat function provided by the instructor
// used to get the unsigned FAT value at an index
uint16_t get_fat_unsigned(uint16_t i) {
   unsigned int j;
   
   if (fat_table == NULL || fat_size == 0) {
      /* File system data hasn't loaded yet */
      return -1;
   }
   if (i & 0x01) {
      j = (unsigned int) (1 + i * 3)/2;
      return (uint16_t) (((unsigned char) (fat_table[j-1] & 0xF0) >> 4) + ((unsigned char) fat_table[j] << 4));
   } else {
      j = (unsigned int) i * 3 / 2;
      return (uint16_t) (((((unsigned char) fat_table[j+1]) & 0x0F) << 8) + ((unsigned char) fat_table[j]));
   }
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

// copies contents of file in disk to a new file
void getContentsOfFile(FILE* fileSystemPtr, uint16_t logicalCluster, char* fileName, int fileSize) {
    int bytesRead = 0;
    
    // convert each character in the new filename to lowercase
    for (int index = 0; index < strlen(fileName); index++) {
        fileName[index] = tolower(fileName[index]);
    }

    // if the size of the file in the disk is initially smaller than 512 bytes, write the contents of that file to the new file and immediately return
    FILE* newFile = fopen(fileName, "wb");
    if (fileSize <= 512) {
        unsigned char content[fileSize];
        fseek(fileSystemPtr, (logicalCluster+31)*512, SEEK_SET);
        fread(content, fileSize, 1, fileSystemPtr);
        fwrite(content, fileSize, 1, newFile);
        fclose(newFile);
        printf("File (%s) successfully copied to your directory!\n", fileName);
        return;
    }

    // loop through the file to be copied until we hit a logical cluster > 0xFF8
    while (logicalCluster < 0xFF8) {
        if (bytesRead + 512 >= fileSize) {
            break;
        }
        fseek(fileSystemPtr, (logicalCluster+31)*512, SEEK_SET);
        unsigned char content[512];
        fread(content, sizeof(content), 1, fileSystemPtr);
        bytesRead += 512;
    
        fwrite(content, sizeof(content), 1, newFile);
        logicalCluster = get_fat_unsigned(logicalCluster);
    }

    // write the remaining bytes to a new file
    if (bytesRead != fileSize) {
        unsigned char leftoverBytes[fileSize-bytesRead];
        fread(leftoverBytes, sizeof(leftoverBytes), 1, fileSystemPtr);
        fwrite(leftoverBytes, sizeof(leftoverBytes), 1, newFile);
    }
    printf("File (%s) successfully copied to your directory!\n", fileName);
    fclose(newFile);
}

// loops through the root directory until we encounter a filename the same as the one the user provided
void diskget(FILE* fileSystemPtr, char* fileName) {
    entry_t directoryEntry;

    fseek(fileSystemPtr, (boot_sector.reserved_sectors + boot_sector.fats * boot_sector.sectors_per_fat) * 512, SEEK_SET);
    for (int i = 0; i < boot_sector.root_entries; i++) {
        fread(&directoryEntry, sizeof(directoryEntry), 1, fileSystemPtr);
        if (directoryEntry.filename[0] == 0) {
            break;
        }
        if (directoryEntry.filename[0] == 0xE5) {
            continue;
        }
        if (directoryEntry.attributes == 0x0F || directoryEntry.attributes == 0x08) {
            continue;
        }
        if ((directoryEntry.attributes & 0x10) == 0) {
            char dirName[9];
            strncpy(dirName, directoryEntry.filename, 8);
            addNullTerminatorToString(dirName);

            char fileExtension[4];
            strncpy(fileExtension, directoryEntry.extension,3);
            addNullTerminatorToString(fileExtension);

            char fullFileName[20];
            strcpy(fullFileName, "");
            strcat(fullFileName, dirName);
            strcat(fullFileName, ".");
            strcat(fullFileName, fileExtension);

            // compare the user provided filename and directory entry filename (ignoring case)
            if (strcasecmp(fullFileName, fileName) == 0) {
                getContentsOfFile(fileSystemPtr, (uint16_t) directoryEntry.cluster, fileName, directoryEntry.size);
                return;
            }
        }

    }
    printf("File not found\n");
}