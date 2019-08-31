#include "main.h"
#include "sdram.h"

#define SDRAM_SIZE				(0x2000000)
#define LOG_BUFF_SIZE			(1 << 20)

typedef struct
{
	unsigned 		is_used;
	char * 			buffer_base;
	unsigned int	buffer_size;
}sdram_buffer_map;

	
int init_alloc_sdram(void);
sdram_buffer_map * alloc_sdram(int index_num);
void free_sdram(sdram_buffer_map * index);

