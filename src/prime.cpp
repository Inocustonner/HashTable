#include "prime.h"
#include "custom_types.h"
#include <math.h>


bool is_prime(const u32 x)
{
	if (x < 2 || x % 2 == 0) return false;
	else if (x < 4) return true;
	else
	{
		for (u32 i = 3; i < (u32)floor(sqrt(x)); i+=2)
		{
			if (x % i == 0) return false;
		}
		
		return true;
	}
}


u32 next_prime(u32 x)
{
	x = x + 1 - (x % 2);
	while (!is_prime(x)) x+= 2;
	
	return x;
}
