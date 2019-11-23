#include <stdio.h>
#include <stdlib.h>
#include "HashTableTemplates.h"

int main()
{
	ht_hash_table<int> *ht = ht_new<int>();
	int *x = (int*)malloc(4);
	*x = 4;
	int *y = (int*)malloc(4);
	*y = 5;
	x = (int*)malloc(4);
	*x = 55;
	ht_insert(ht, "key", sizeof("key"), x);
	ht_insert(ht, "k", sizeof("k"), y);
	ht_insert(ht, "key", sizeof("key"), x);
	printf("%d %d\n", *ht_get(ht, "key", sizeof("key")),
		   *ht_get(ht, "k", sizeof("k")));
	ht_del_hash_table(ht);
	return 0;
}
