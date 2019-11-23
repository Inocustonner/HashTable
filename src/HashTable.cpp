#include "HashTable.h"
#include "custom_types.h"
#include "prime.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define HT_INITIAL_SIZE 53
#define HT_PRIME1 53
#define HT_PRIME2 161
#define TMP_BUFFER_SZ 1024


global_variable char TempBuffer[TMP_BUFFER_SZ];


u32 ht_hash(const char *str, const u32 a, u32 m)
{
	u32 hash = 0;
	u32 len = strlen(str);
	for (u32 i = 0; i < len; ++i)
	{
		hash += (u32)pow(a, len - (i + 1)) * str[i];
		hash = hash % m;
	}
	return hash;
}


u32 ht_get_hash(const char *key, const u32 nBuckets, const u32 attempt)
{
	const u32 hash_a = ht_hash(key, HT_PRIME1, nBuckets);
	const u32 hash_b = ht_hash(key, HT_PRIME2, nBuckets);
	return (hash_a + attempt * (hash_b + 1)) % nBuckets;
}


global_variable ht_item HT_DELETED_ITEM = { "", NULL };
void ht_del_item(ht_item *item);

ht_item *ht_new_item(const char *key, const u32 len,
					 void *val, u32 val_size)
{
	ht_item *item = (ht_item*)calloc(1, sizeof(ht_item));
	
	item->key = (const char*)calloc(len + 1, sizeof(char));// key is null terminated
	memcpy((void*)item->key, (void*)key, len);
		
	item->value = val;

	item->size = val_size;
	return item;
}


ht_item *ht_new_item_copy(const char *key, const u32 len,
						  void *val, u32 val_size)
{
	ht_item *item = (ht_item*)calloc(1, sizeof(ht_item));

   	item->key = (const char*)calloc(len + 1, sizeof(char));
	memcpy((void*)item->key, (void*)key, len);
	
	item->value = (void*)calloc(1, val_size);
	memcpy((void*)item->value, (void*)val, val_size);

	item->size = val_size;
	return item;
}


void ht_resize(ht_hash_table *ht, u32 base_size)
{
	if (base_size < HT_INITIAL_SIZE)
		return;

	ht_hash_table *ht_new = ht_new_sized(base_size);
	for (u32 i = 0; i < ht->size; ++i)
	{
		ht_item *item = ht->items[i];
		if (item != NULL && item != &HT_DELETED_ITEM)
		{
			ht_item *item_copy = ht_new_item_copy(item->key, strlen(item->key),
									  item->value, item->size);
			ht_insert_item(ht_new, item_copy);
		}
	}

	ht->count = ht_new->count;

	//swap sizes
	u32 tmp_size = ht->size;
	ht->size = ht_new->size;
	ht_new->size = tmp_size;

	//swap items
	ht_item **tmp_items = ht->items;
	ht->items = ht_new->items;
	ht_new->items = tmp_items;

	ht_del_hash_table(ht_new);
}


void ht_resize_up(ht_hash_table *ht)
{
	u32 new_size = ht->size * 2;
	ht_resize(ht, new_size);
}


void ht_resize_down(ht_hash_table *ht)
{
	u32 new_size = ht->size / 2;
	ht_resize(ht, new_size);
}


void ht_insert(ht_hash_table *ht, const char *key, u32 len,
			   void *value, u32 val_size)
{
	u32 load = ht->count * 100 / ht->size;
	if (load > 70)
	{
		ht_resize_up(ht);
	}
	else if (load < 10)
	{
		ht_resize_down(ht);
	}
	
	ht_item *item_new = ht_new_item(key, len, value, val_size);	
	u32 index = ht_get_hash(item_new->key, ht->size, 0);

	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		//if we need to reinsert
		ht_item *curr_item = ht->items[index];
		if (curr_item != &HT_DELETED_ITEM)
		{
			if (strcmp(item_new->key, curr_item->key) == 0)
			{
				ht_del_item(curr_item);
				ht->items[index] = item_new;
				return;
			}
		}
		index = ht_get_hash(item_new->key, ht->size, i);
	}

	ht->items[index] = item_new;
	ht->count++;
}


void ht_insert_item(ht_hash_table *ht, ht_item *item)
{
	u32 index = ht_get_hash(item->key, ht->size, 0);

	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		//if we need to reinsert
		ht_item *curr_item = ht->items[index];
		if (curr_item != &HT_DELETED_ITEM)
		{
			if (strcmp(item->key, curr_item->key) == 0)
			{
				ht_del_item(curr_item);
				ht->items[index] = item;
				return;
			}
		}
		index = ht_get_hash(item->key, ht->size, i);
	}

	ht->items[index] = item;
	ht->count++;
}


char *make_terminated(const char *str, u32 len)
{
	if (len < TMP_BUFFER_SZ)
	{
		memset((void*)TempBuffer, '\0', len + 1);
		memcpy((void*)TempBuffer, (void*)str, len);
	    return TempBuffer;
	}
	else
	{
		char* str_allocated = (char*)calloc(len + 1, sizeof(char));
		memcpy((void*)str_allocated, (void*)str, len);
		return str_allocated;
	}
}


void *ht_get(ht_hash_table *ht, const char *key, u32 len)
{
	const char *key_terminated = (const char*)make_terminated(key, len);
	
	u32 index = ht_get_hash(key_terminated, ht->size, 0);

	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		if (ht->items[index] != &HT_DELETED_ITEM &&
			strcmp(key_terminated, ht->items[index]->key) == 0)
		{
			return ht->items[index]->value;
		}

		index = ht_get_hash(key_terminated, ht->size, i);
	}

	//if was allocated by the function make_terminated
	if ((void*)key_terminated != (void*)TempBuffer)
	{
		free((void*)key_terminated);
	}
	return NULL;
}


void ht_delete(ht_hash_table *ht, const char *key, u32 len)
{
	const char *key_terminated = (const char*)make_terminated(key, len);

	u32 index = ht_get_hash(key_terminated, ht->size, 0);
	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		ht_item *item = ht->items[index];
		if (item != &HT_DELETED_ITEM)
		{
			if (strcmp(key_terminated, item->key) == 0)
			{
				ht_del_item(item);
				ht->items[index] = &HT_DELETED_ITEM;
				break;
			}
		}

		index = ht_get_hash(key_terminated, ht->size, i);
	}
	ht->count--;
}


ht_hash_table *ht_new_sized(const u32 base_size)
{
	ht_hash_table *ht_new = (ht_hash_table*)calloc(1, sizeof(ht_hash_table));
	
	ht_new->size = next_prime(base_size);
	ht_new->items = (ht_item**)calloc(ht_new->size, sizeof(ht_item*));
	return ht_new;
}


ht_hash_table *ht_new()
{
	return ht_new_sized(HT_INITIAL_SIZE);
}


void ht_del_item(ht_item *item)
{
	free((void*)item->key);
	free((void*)item->value);
	free((void*)item);
}


void ht_del_hash_table(ht_hash_table *ht)
{
	for (u32 i = 0; i < ht->count; ++i)
	{
		if (ht->items[i] != NULL && ht->items[i] != &HT_DELETED_ITEM)
		    ht_del_item(ht->items[i]);
	}
	free((void*)ht->items);
	free((void*)ht);
}
