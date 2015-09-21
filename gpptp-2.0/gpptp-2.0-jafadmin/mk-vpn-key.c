#include <stdio.h>
#include <string.h>
#include <stdlib.h> 		// for getenv()
#include <unistd.h> 		// for access()
#include <sys/stat.h> 	// for mkdir()


void encrypt(char *array, int array_size)
{
    int i;
    char secret[] = "t|!p(`$v6pP&+^x~$T'2{#~?P)4.=l"; //30 character key
    for(i = 0; i < array_size; i++)
        array[i] ^= secret[i];
}

int main(void)
{		
	
	FILE  *InFile, *OutFile;
	char str[26], str2[26], KeyPath[256];	
	
	strcpy(KeyPath, getenv("HOME"));
	strcat(KeyPath, "/.gpptp"); 				// Make a path to the user's home directory (for caching)  
  	if(access(KeyPath , F_OK) != 0)  		// does "/root/.gpptp" exist?
  		mkdir (KeyPath , 0755);            		 	// if not, create it.
		
	strcat(KeyPath, "/gpptpkey.0"); 
	
	printf("\t            Input key:  "); gets(str);
		
	printf("\t    Before saving key: >%s<\n" ,str);
	OutFile = fopen (KeyPath , "w+");
	encrypt(str, sizeof(str));	
	fputs(str, OutFile);
	fclose(OutFile);
	
	InFile = fopen (KeyPath , "r");
	fgets(str2,  sizeof(str2) - 1, InFile);
	printf("\tAfter reading keyfile: >%s<\n" ,str2);
	
	encrypt(str2, sizeof(str2));
	
	printf("\t     After decryption: >%s<\n\n" ,str2);
	
	fclose(InFile);	
	
	exit(0);
			
}