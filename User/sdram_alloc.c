#include "sdram_alloc.h"

osMutexDef_t g_sdram_mutex;
osMutexId	g_sdram_mutex_id;

sdram_buffer_map g_sdram_map[] = 
{
	{0, (char *)Bank5_SDRAM_ADDR, LOG_BUFF_SIZE},
	{0, (char *)(Bank5_SDRAM_ADDR + LOG_BUFF_SIZE), 		LOG_BUFF_SIZE},
	{0, (char *)(Bank5_SDRAM_ADDR + LOG_BUFF_SIZE * 2), 	LOG_BUFF_SIZE},
	{0, (char *)(Bank5_SDRAM_ADDR + LOG_BUFF_SIZE * 3), 	LOG_BUFF_SIZE},
	{0, (char *)(Bank5_SDRAM_ADDR + LOG_BUFF_SIZE * 4), 	LOG_BUFF_SIZE},
};


int init_alloc_sdram(void)
{
	int i;
	unsigned int total_count = 0;
	for(i = 0; i < sizeof(g_sdram_map)/sizeof(g_sdram_map[0]); ++i)
	{
		total_count += g_sdram_map[i].buffer_size;
		if(total_count > SDRAM_SIZE)
		{
			return -1;
		}
		g_sdram_map[i].is_used = 0;
	}

	g_sdram_mutex_id = osMutexCreate(&g_sdram_mutex);

	return 0;
}

sdram_buffer_map * alloc_sdram(int index_num)
{
	int i;
	sdram_buffer_map * index = NULL;

	if(osOK == osMutexWait(g_sdram_mutex_id, 0))
	{
		if(index_num < 0)
		{
			for(i = 0; i < sizeof(g_sdram_map)/sizeof(g_sdram_map[0]); ++i)
			{
				if(0 == g_sdram_map[i].is_used)
				{
					g_sdram_map[i].is_used = 1;
					index = &g_sdram_map[i];				
					break;
				}
			}
		}
		else
		{
			if(0 == g_sdram_map[index_num].is_used)
			{
				g_sdram_map[i].is_used = 1;
				index = &g_sdram_map[index_num];
			}
		}

		osMutexRelease(g_sdram_mutex_id);
	}
	
	return index;
}

void free_sdram(sdram_buffer_map * index)
{
	osMutexWait(g_sdram_mutex_id, 0);
	(*index).is_used = 0;
	osMutexRelease(g_sdram_mutex_id);
}
