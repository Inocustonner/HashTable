#pragma once

#include "custom_types.h"
#include "prime.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define HT_INITIAL_SIZE 53
#define HT_PRIME1 83
#define HT_PRIME2 43
#define TMP_BUFFER_SZ 1024


global_variable char TempBuffer[TMP_BUFFER_SZ];

internal
u32 ht_hash(const char *str, const u32 a, u32 m)
{
	u32 hash = 0;
	u32 len = (u32)strlen(str);
	for (u32 i = 0; i < len; ++i)
	{
		hash += (u32)pow(a, len - (i + 1)) * str[i];
		hash = hash % m;
	}
	return hash;
}

internal
u32 ht_get_hash(const char *key, const u32 nBuckets, const u32 attempt)
{
	const u32 hash_a = ht_hash(key, HT_PRIME1, nBuckets);
	const u32 hash_b = ht_hash(key, HT_PRIME2, nBuckets);
	return (hash_a + attempt * (hash_b + 1)) % nBuckets;
}


template<typename T>
struct ht_item
{
	const char *key;
	T *value;
};


template<typename T>
struct ht_hash_table
{
	unsigned __int32 size;
	unsigned __int32 count;
	ht_item<T> **items;
};


global_variable ht_item<void*> HT_DELETED_ITEM = { "", NULL };


template<typename T>
void ht_del_item(ht_item<T> *item);


template<typename T>
ht_item<T> *ht_new_item(const char *key, const u32 len,
					  T *val)
{
	ht_item<T> *item = (ht_item<T>*)calloc(1, sizeof(ht_item<T>));
	
	item->key = (const char*)calloc(len + 1, sizeof(char));// key is null terminated
	memcpy((void*)item->key, (void*)key, len);
		
	item->value = val;

	return item;
}

template<typename T>
ht_item<T> *ht_new_item_copy(const char *key, const u32 len,
						  T *val)
{
	ht_item<T> *item = (ht_item<T>*)calloc(1, sizeof(ht_item<T>));

   	item->key = (const char*)calloc(len + 1, sizeof(char));
	memcpy((void*)item->key, (void*)key, len);
	
	item->value = (T*)calloc(1, sizeof(T));
	memcpy((void*)item->value, (void*)val, sizeof(T));

	return item;
}

template<typename T>
void ht_resize(ht_hash_table<T> *ht, u32 base_size)
{
	if (base_size < HT_INITIAL_SIZE)
		return;

	ht_hash_table<T> *ht_new = ht_new_sized<T>(base_size);
	for (u32 i = 0; i < ht->size; ++i)
	{
		ht_item<T> *item = ht->items[i];
		if (item != NULL && (void*)item != &HT_DELETED_ITEM)
		{
			ht_item<T> *item_copy = ht_new_item_copy(item->key, (u32)strlen(item->key),
													 item->value);
			ht_insert_item(ht_new, item_copy);
		}
	}

	ht->count = ht_new->count;

	//swap sizes
	u32 tmp_size = ht->size;
	ht->size = ht_new->size;
	ht_new->size = tmp_size;

	//swap items
	ht_item<T> **tmp_items = ht->items;
	ht->items = ht_new->items;
	ht_new->items = tmp_items;

	ht_del_hash_table(ht_new);
}

template<typename T>
void ht_resize_up(ht_hash_table<T> *ht)
{
	u32 new_size = ht->size * 2;
	ht_resize(ht, new_size);
}

template<typename T>
void ht_resize_down(ht_hash_table<T> *ht)
{
	u32 new_size = ht->size / 2;
	ht_resize(ht, new_size);
}

template<typename T>
void ht_insert(ht_hash_table<T> *ht, const char *key, u32 len,
			   T *value)
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
	
	ht_item<T> *item_new = ht_new_item(key, len, value);	
	u32 index = ht_get_hash(item_new->key, ht->size, 0);

	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		//if we need to reinsert
		ht_item<T> *curr_item = ht->items[index];
		if ((void*)curr_item != &HT_DELETED_ITEM)
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

template<typename T>
void ht_insert_item(ht_hash_table<T> *ht, ht_item<T> *item)
{
	u32 index = ht_get_hash(item->key, ht->size, 0);

	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		//if we need to reinsert
		ht_item<T> *curr_item = ht->items[index];
		if ((void*)curr_item != &HT_DELETED_ITEM)
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

internal
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

template<typename T>
T *ht_get(ht_hash_table<T> *ht, const char *key, u32 len)
{
	const char *key_terminated = (const char*)make_terminated(key, len);
	
	u32 index = ht_get_hash(key_terminated, ht->size, 0);

	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		if ((void*)ht->items[index] != &HT_DELETED_ITEM &&
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

template<typename T>
void ht_delete(ht_hash_table<T> *ht, const char *key, u32 len)
{
	const char *key_terminated = (const char*)make_terminated(key, len);

	u32 index = ht_get_hash(key_terminated, ht->size, 0);
	for (u32 i = 1; ht->items[index] != NULL; ++i)
	{
		ht_item<T> *item = ht->items[index];
		if (item != &HT_DELETED_ITEM)
		{
			if (strcmp(key_terminated, item->key) == 0)
			{
				ht_del_item(item);
				ht->items[index] = (T*)&HT_DELETED_ITEM;
				ht->count--;
				break;
			}
		}

		index = ht_get_hash(key_terminated, ht->size, i);
	}
}

template<typename T>
ht_hash_table<T> *ht_new_sized(const u32 base_size)
{
	ht_hash_table<T> *ht_new = (ht_hash_table<T>*)calloc(1, sizeof(ht_hash_table<T>));
	
	ht_new->size = next_prime(base_size);
	ht_new->items = (ht_item<T>**)calloc(ht_new->size, sizeof(ht_item<T>*));
	return ht_new;
}

template<typename T>
ht_hash_table<T> *ht_new()
{
	return ht_new_sized<T>(HT_INITIAL_SIZE);
}

template<typename T>
void ht_del_item(ht_item<T> *item)
{
	free((void*)item->key);
	free((void*)item->value);
	free((void*)item);
}

template<typename T>
void ht_del_hash_table(ht_hash_table<T> *ht)
{
	for (u32 i = 0; i < ht->count; ++i)
	{
		if (ht->items[i] != NULL && (void*)ht->items[i] != &HT_DELETED_ITEM)
		    ht_del_item(ht->items[i]);
	}
	free((void*)ht->items);
	free((void*)ht);
}
