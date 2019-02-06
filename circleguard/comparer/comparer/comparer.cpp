// comparer.cpp : Defines the exported functions for the DLL application.
//

#include "header.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "comparer.h"
#include "LzmaDec.h"

constexpr auto PROP_OFFSET = 0U;
constexpr auto PROP_SIZE = 5U;
constexpr auto USIZE_OFFSET = 5U;
constexpr auto USIZE_SIZE = 8U;
constexpr auto DATA_OFFSET = 13U;

uint8_t b64lut[] = {
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
	 0,  0,  0, 62,  0,  0,  0, 63, 52, 53, 
	54, 55, 56, 57, 58, 59, 60, 61,  0,  0, 
	 0,  0,  0,  0,  0,  0,  1,  2,  3,  4, 
	 5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
	25,  0,  0,  0,  0,  0,  0, 26, 27, 28, 
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 
	49, 50, 51
};

union b64seg {
	uint8_t byt[4];
	int32_t val;
};

size_t b64decode(uint8_t *s, size_t l) {
	if (l % 4 != 0) {
		return 0;
	}

	size_t max = l / 4;

	size_t decLen = max * 3;
	if (s[l - 1] == '=') {
		if (s[l - 2] == '=') {
			decLen -= 2;
		}
		else {
			decLen -= 1;
		}
	}

	for (size_t i = 0; i < max; i++) {
		b64seg t;

		t.val = 
			b64lut[s[i * 4]] << 18 | 
			b64lut[s[i * 4 + 1]] << 12 | 
			b64lut[s[i * 4 + 2]] << 6 | 
			b64lut[s[i * 4 + 3]];

		s[i * 3] = t.byt[2];
		s[i * 3 + 1] = t.byt[1];
		s[i * 3 + 2] = t.byt[0];
	}

	return decLen;
}

void print_lzma_error(int e, ELzmaStatus status) {
	if (e == SZ_OK) {
		if (status == LZMA_STATUS_FINISHED_WITH_MARK) {
			printf("LZMA decode finished with mark.\n");
		}
		else if (status == LZMA_STATUS_NOT_FINISHED) {
			printf("LZMA decode did not finish.\n");
		}
		else if (status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK) {
			printf("LZMA decode may have finished without mark.\n");
		}
		else {
			printf("LZMA decode status unknown.\n");
		}
	}
	else {
		printf("ERROR: ");
		if (e == SZ_ERROR_DATA) {
			printf("Data error.\n");
		}
		else if (e == SZ_ERROR_MEM) {
			printf("Memory allocation error.\n");
		}
		else if (e == SZ_ERROR_UNSUPPORTED) {
			printf("Unsupported properties.\n");
		}
		else if (e == SZ_ERROR_INPUT_EOF) {
			printf("Input reached EOF too soon.\n");
		}
		else {
			printf("Unknown.\n");
		}
	}
}

void *SzAlloc(ISzAllocPtr p, size_t size) { p = p; return malloc(size); }
void SzFree(ISzAllocPtr p, void *address) { p = p; free(address); }

size_t lzmaDecode(uint8_t *s, size_t l, char **d) {
	ISzAlloc alloc = { SzAlloc, SzFree };

	const Byte *propData = s + PROP_OFFSET;

	SizeT srcLen = (l - DATA_OFFSET);
	const Byte *src = s + DATA_OFFSET;

	SizeT destLen;

	int64_t size = *((int64_t *)(s + USIZE_OFFSET));
	if (size != -1) {
		destLen = size;
	}
	else {
		destLen = (l - DATA_OFFSET) * 2;
	}

	Byte *dest = (Byte *)malloc(destLen);

	ELzmaStatus status;

	int e = LzmaDecode(dest, &destLen, src, &srcLen, propData, 5U, LZMA_FINISH_ANY, &status, &alloc);

	print_lzma_error(e, status);

	*d = (char *)dest;
	return destLen;
}

static size_t countPoints(char *s, size_t l) {
	size_t c = 0;
	for (size_t i = 0; i < l; i++) {
		if (s[i] == ',') c++;
	}
	return c;
}

int cmp(const void *a, const void *b) {
	if (((point *)a)->time <  ((point *)b)->time) return -1;
	if (((point *)a)->time == ((point *)b)->time) return  0;
	return 1;
}

size_t parseReplay(char *s, size_t l, point **pArrP) {
	size_t pCount = countPoints(s, l);
	point *pArr = (point *)malloc(pCount * sizeof(point));

	uint64_t time = 0;

	for (size_t i = 0; i < pCount; i++) {
		time += atoll(s);
		pArr[i].time = time;

		while (*(s++) != '|');
		pArr[i].x = atof(s);

		while (*(s++) != '|');
		pArr[i].y = atof(s);

		while (*(s++) != ',');
	}

	qsort((void *)pArr, pCount, sizeof(point), cmp);
	*pArrP = pArr;
	return pCount;
}

static inline void inc(point *f, point *b, point *s, size_t *i) {
	*b = *f;
	*f = s[++(*i)];
}

static inline void interp(point *f, point *b, uint64_t t, point *p) {
	double r;
	if (f->time == b->time) {
		r = 0;
	}
	else {
		r = ((double)(t - b->time)) / (f->time - b->time);
	}
	p->time = t;
	p->x = b->x + (f->x - b->x) * r;
	p->y = b->y + (f->y - b->y) * r;
}

static inline double dist(point *a, point *b) {
	double delX = a->x - b->x;
	double delY = a->y - b->y;
	return sqrt(delX * delX + delY * delY);
}

static inline void compInterp(point *b, point *f, point *i, double *sum, size_t *n) {
	point p;
	interp(f, b, i->time, &p);

	*sum += dist(i, &p);
	(*n)++;
}

double compare(point *s1, size_t l1, point *s2, size_t l2) {
	if (l1 < 2 || l2 < 2) return 0;

	point f1, b1, f2, b2;
	b1 = s1[0];
	f1 = s1[1];
	b2 = s2[0];
	f2 = s2[1];

	double sum = 0.0;
	size_t n = 0;
	size_t i1, i2;
	i1 = i2 = 1;

	while (i1 < l1 && i2 < l2) {
		if (b1.time < b2.time) {
			if (f1.time < b2.time) {
				inc(&f1, &b1, s1, &i1);
			}
			else {
				if (f1.time < f2.time) {
					compInterp(&b1, &f1, &b2, &sum, &n);
					compInterp(&b2, &f2, &f1, &sum, &n);
					inc(&f1, &b1, s1, &i1);
				}
				else {
					compInterp(&b1, &f1, &b2, &sum, &n);
					compInterp(&b1, &f1, &f2, &sum, &n);
					inc(&f2, &b2, s2, &i2);
				}
			}
		}
		else {
			if (b1.time > f2.time) {
				inc(&f2, &b2, s2, &i2);
			}
			else {
				if (f1.time > f2.time) {
					compInterp(&b2, &f2, &b1, &sum, &n);
					compInterp(&b1, &f1, &f2, &sum, &n);
					inc(&f2, &b2, s2, &i2);
				}
				else {
					compInterp(&b2, &f2, &b1, &sum, &n);
					compInterp(&b2, &f2, &f1, &sum, &n);
					inc(&f1, &b1, s1, &i1);
				}
			}
		}
	}

	return sum / n;
}
