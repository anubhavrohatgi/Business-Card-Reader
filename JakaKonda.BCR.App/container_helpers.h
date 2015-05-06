#ifndef BCR_CONTAINER_HELPERS_H
#define BCR_CONTAINER_HELPERS_H

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

template <typename T>
inline void remove_if_exists(std::vector<T> &vec, T obj)
{
	auto it = vec.begin();

	while (it != vec.end())
	{
		if (obj == *it)
		{
			vec.erase(it);
			return;
		}

		++it;
	}
}

BCR_END_NAMESPACE

#endif // BCR_CONTAINER_HELPERS_H