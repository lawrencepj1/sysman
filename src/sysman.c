/* This is a tool to synchronize multiple computers. It generates status files from a source computer which can be compared */
/* to status files on a target computer to create archive files. The archive files contain a list of the files that have been */
/* updated on the source computer but not on a target computer. The archive files can be used to update all the files on the */
/* target computer using the files stored in the archive directory. */
/* Author Peter Lawrence - Terrestrial Sciences Section - National Center for Atmospheric Research */
/* Email:  lawrence@ucar.edu */ 
/* Web:    https://www.cgd.ucar.edu/staff/lawrence */
/* GitHub: https://github.com/lawrencepj1 */
/* Phone:  +1 303-4971727 (work) +1 303-9566932 (mobile) */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

char hostname[1024];
char localroot[1024];
char adminroot[1024];
char statusroot[1024];
char archiveroot[1024];

struct machinenode {
    int machinenumber;
    char machinename[1024];
    struct machinenode *nextmachine;
};

int machinenum = 0;
struct machinenode *machinelisthead = NULL;

struct machinenode* addmachinenode(struct machinenode *machinelist,int machinenumber,char *machinename) {

    struct machinenode *newmachinelisthead;
    struct machinenode *newmachinenode;
    struct machinenode *currentmachinenode; 
    struct machinenode *nextmachinenode;

    newmachinenode = (struct machinenode*) malloc(sizeof(struct machinenode));
    newmachinenode->machinenumber = machinenumber;
    sprintf(newmachinenode->machinename,"%s",machinename);
    
    if (machinelist != NULL) {
        currentmachinenode = machinelist;
	nextmachinenode = currentmachinenode->nextmachine;
	while (nextmachinenode != NULL && nextmachinenode->machinenumber < machinenumber) {
	    currentmachinenode = nextmachinenode;
	    nextmachinenode = currentmachinenode->nextmachine;
	}
	currentmachinenode->nextmachine = newmachinenode;
	newmachinenode->nextmachine = nextmachinenode;
	newmachinelisthead = machinelist;
    }
    else {
        newmachinelisthead = newmachinenode;
    }
    
    machinenum++;
    
    return newmachinelisthead;
    
}


int printmachinenodes(struct machinenode *machinelist) {

    struct machinenode *currentmachinenode; 

    currentmachinenode = machinelist;
    while (currentmachinenode != NULL) {
        printf("%d %s\n",currentmachinenode->machinenumber,currentmachinenode->machinename);
	currentmachinenode = currentmachinenode->nextmachine;
    }

    return 0;
    
}


int readconfigfile(char *homeline) {

    DIR *dirhome;
    struct dirent * dircontents;
    
    FILE *configfile;
    char configfilename[1024];
    char fieldname[1024];
    
    int configfileexists;
    
    dirhome = opendir(homeline);
    if (dirhome == NULL) {
        printf("Error: configfile .sysman not found in %s\n",homeline);
	return 1;
    }
    
    configfileexists = 0;
    while ((dircontents = readdir(dirhome)) != NULL) {
        if (strcmp(dircontents->d_name,".sysman") == 0) {
	    configfileexists = 1;
	}
    }
    
    if (configfileexists == 0) {
        printf("Error: configfile .sysman not found in %s\n",homeline);
	return 1;
    }
    
    sprintf(configfilename,"%s/.sysman",homeline);
    configfile = fopen(configfilename,"r");

    fscanf(configfile,"%s %s",fieldname,hostname);
    fscanf(configfile,"%s %s",fieldname,localroot);
    sprintf(adminroot,"%sadmin",localroot);
    sprintf(statusroot,"%sstatus",localroot);
    sprintf(archiveroot,"%sarchive",localroot);
    
    printf("Running sysman for hostname %s with local root %s\n",hostname,localroot);
     
    return 0;

}


int readmachinelist(char *adminroot) {

    DIR *dirhome;
    struct dirent * dircontents;
    
    FILE *machinefile;
    char machinefilename[1024];
    char fieldname[1024];
    
    int machineid;
    char machinename[1024];
    int machinefileexists;
    
    dirhome = opendir(adminroot);
    if (dirhome == NULL) {
        printf("Error: machinelist not found in %s\n",adminroot);
	return 1;
    }
    
    machinefileexists = 0;
    while ((dircontents = readdir(dirhome)) != NULL) {
        if (strcmp(dircontents->d_name,"machinelist.txt") == 0) {
	    machinefileexists = 1;
	}
    }
    
    if (machinefileexists == 0) {
        printf("Error: machinelist not found in %s\n",adminroot);
	return 1;
    }
    
    sprintf(machinefilename,"%s/machinelist.txt",adminroot);
    machinefile = fopen(machinefilename,"r");

    while (fscanf(machinefile,"%d %s",&machineid,machinename) != EOF) {
        machinelisthead = addmachinenode(machinelisthead,machineid,machinename);
    }
    
    printmachinenodes(machinelisthead);
    
    return 0;
    
}


int main(long narg, char **argv, char **envp) {

    char **env;
    char envline[1024];
    char *envtoken;
    char envtag[1024];
    char homeline[1024];

    if (narg == 1) {
        printf("Usage: sysman command [arguments]\n");
        printf("Commands: \n");
	printf("           machine register machinename \n");
	printf("           machine list \n");
	printf("           datalocation register datalocationname datadirectory \n");
	printf("           datalocation list \n");
	printf("           status run datalocation \n");
	printf("           status list datalocation \n");
	printf("           status import statusfilename \n");
	printf("           archive run sourcestatus# targetstatus# \n");
	printf("           archive list datalocation \n");
	printf("           archive import archivefilename \n");
	printf("           update run archive# \n");
	printf("           update list datalocation\n");
	
	return 1;
    }

    env = envp;
    while(*env != 0) {
        sprintf(envline,"%s",*env);    
	envtoken = strtok(envline, "=");
	sprintf(envtag,"%s",envtoken);
	if (strcmp(envtag,"HOME") == 0) {
            envtoken = strtok(NULL,"=");
            sprintf(homeline,"%s",envtoken);
	    *env = 0;
	}
	else {
            env++;
	}
    }
        
    if (readconfigfile(homeline) != 0) {
        return 1;
    }
    
    if (readmachinelist(adminroot) != 0) {
        return 1;
    }

    return 0;
    
}
