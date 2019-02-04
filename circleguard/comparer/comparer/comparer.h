#pragma once

#ifdef COMPARER_EXPORTS
#define COMPARER_API __declspec(dllexport)
#else
#define COMPARER_API __declspec(dllimport)
#endif

extern "C" COMPARER_API size_t b64decode(uint8_t *s, size_t l);

extern "C" COMPARER_API size_t lzmaDecode(uint8_t *s, size_t l, uint8_t **d);
