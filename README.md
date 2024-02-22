# SFS (Computer Science 360)

Omar Madhani

Sources: Some of the code used in my program was provided by the course instructor Ahmad Abdullah. I also modified some of this code.

(Please look in diskfunctions.h for further details)

Ensure that diskinfo.c, disklist.c, diskget.c, diskfunctions.h, the Makefile, and any test disk image files (FAT-12) are in the same Linux directory.

To compile the four C files and create four executables, simply type "make" in your command line (within the same directory).

Assume you wanted to run the programs on a disk image "disk.IMA" and a file "testFile.txt".

Part 1: 
To run the executable "diskinfo" with the filenames above, type the following in your command line: 

	./diskinfo disk.IMA

diskinfo will display the following information about the disk image: OS Name, the label of the disk, the total size of the disk, free size of the disk, number of files in the disk, number of FAT copies, and sectors per FAT.
NOTE: If the disk image does not have a label, the program will print the line "Label of the disk: NO NAME". My program also does not consider long file names or the disk label as a file.

Part 2: 
To run the executable "disklist" with the filenames above, type the following in your command line:
	
	./disklist disk.IMA

disklist will print out the directories and files within a disk image. Directories are denoted by "D" and files are denoted by "F". 
You will see information about each file, including its size and creation date/time. Disklist should print up to 16 files/subdirectories per directory.    

Part 3: 
To run the executable "diskget" with the filenames above, type the following in your command line:
	
	./diskget disk.IMA testFile.txt

diskget will copy a file in the root directory of a disk image to your Linux directory. Please note that the case of the filename you provide does not matter.
In other words, as long as the filename you provide matches one in the root directory, ignoring case, diskget will work as intended. 
If the filename you provide does not match any of those in the root directory of the disk image, you will see a "File not found" error.

You will receive a confirmation that the file has been copied to your Linux directory. Please note that the filename of the copied file will be in lowercase.
To view the actual content of a formatted file (e.g. PDF/JPG) copied from the disk image, you may have to open the file in another program, like a PDF viewer.
However, other types of files, like .tex, .py, and .txt can be viewed by typing:	

cat filename.extension

Replace "disk.IMA" and "testFile.txt" with any other disk image and file of choice. Ensure that the number of parameters is the same as in the examples above. 
Otherwise, an error message will be displayed.

Thank you!

