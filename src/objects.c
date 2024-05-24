#include "objects.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Object *object_append(Object **list)
{
	assert(list);
	Object *obj = malloc(sizeof(*obj));
	memset(obj, 0, sizeof(*obj));

	if (*list == NULL) {
		*list = obj;
		return obj;
	}

	for (Object *it = *list; it; it = it->next) {
		if (it->next == NULL) {
			it->next = obj;
			return obj;
		}
	}

	// should never happen
	return NULL;
}

Object *object_insert(Object **list)
{
	assert(list);
	Object *obj = malloc(sizeof(*obj));
	memset(obj, 0, sizeof(*obj));
	obj->next = *list;
	(*list) = obj;
	return obj;
}

Object *object_find(Object **list, const char *name)
{
	assert(list);
	assert(*list);

	for (Object *it = *list; it; it = it->next) {
		if (string_equal(it->name, name)) {
			return it;
		}
	}

	return NULL;
}
