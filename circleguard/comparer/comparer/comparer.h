#pragma once

#ifdef COMPARER_EXPORTS
#define COMPARER_API __declspec(dllexport)
#else
#define COMPARER_API __declspec(dllimport)
#endif

typedef struct point {
	uint64_t time;
	double x, y;
} point;

extern "C" COMPARER_API size_t b64decode(uint8_t *s, size_t l);

extern "C" COMPARER_API size_t lzmaDecode(uint8_t *s, size_t l, char **d);

extern "C" COMPARER_API size_t parseReplay(char *s, size_t l, point **pArrP);

extern "C" COMPARER_API double compare(point *s1, size_t l1, point *s2, size_t l2);
