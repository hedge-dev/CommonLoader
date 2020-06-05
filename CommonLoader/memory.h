#pragma once

void memcpy(void* destination, void* source, size_t length) 
{
	for (size_t i = 0; i < length; i++) 
	{
		((char*)destination)[i] = ((char*)source)[i];
	}
}