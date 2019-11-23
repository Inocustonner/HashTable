//IMPORTANT NOTE !!!
// "val" element that is passed to the ht_new_item will not be copied the pointer passed
// will be just copied. If you want to copy consider using the copy version
// of the function.
//IMPORTANT NOTE !!!
#pragma once

typedef struct
{
	const char *key;
	void *value;
	unsigned __int32 size;
}ht_item;

typedef struct
{
	unsigned __int32 size;
	unsigned __int32 count;
	ht_item **items;
}ht_hash_table;

ht_item *ht_new_item(const char *key, const unsigned __int32 len,
					 void *val, unsigned __int32 val_size);
ht_item *ht_new_item_copy(const char *key, const unsigned __int32 len,
						  void *val, unsigned __int32 val_size);

void ht_insert(ht_hash_table *ht, const char *key, unsigned __int32 len,
			   void *value, unsigned __int32 val_size);
void ht_insert_item(ht_hash_table *ht, ht_item *item);
void *ht_get(ht_hash_table *ht, const char *key, unsigned __int32 len);
void ht_delete(ht_hash_table *ht, const char *key, unsigned __int32 len);

ht_hash_table *ht_new_sized(const unsigned __int32 base_size);
ht_hash_table *ht_new();
void ht_del_hash_table(ht_hash_table* ht);
