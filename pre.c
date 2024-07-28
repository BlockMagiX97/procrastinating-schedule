// this file includes auto generation for struct related functions (write, read, free)
// free_(type) frees all pointer references, BUT THE STRUCT ITSELF MUST BE FREED !!!!!!!!!!!!!!
// free does NOT support pointers to pointers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/socket.h>

#ifndef STRUCT_AUTO
	#define STRUCT_AUTO
	#define STRUCTS(_F, ...) \
		_F(RECORD, record_t, record, __VA_ARGS__) \

	#define RECORD_FIELDS(_F, ...) \
		_F(uint8_t	,	, NORMAL, permission	,	, __VA_ARGS__) \
		_F(uint16_t	,	, NORMAL, owner		,	, __VA_ARGS__) \
		_F(uint16_t	,	, NORMAL, group		,	, __VA_ARGS__) \
		_F(uint8_t	,	, NORMAL, task_len	,	, __VA_ARGS__) \
		_F(char		,   *	, PTR	, task		, task_len, __VA_ARGS__) \
		_F(time_t	,	, NORMAL, finish	,	, _VA_ARGS__) \
		_F(time_t	,	, NORMAL, start		,	, _VA_ARGS__) \
		_F(time_t	,	, NORMAL, to_do		,	, _VA_ARGS__)



	// not auto generated because of external types
	#define FREE_STRUCT_TABLE(X) _Generic((X), \
		struct record_t *: free_record_t, \
		default: free \
	)(X);

	#define SERIALIZE_STRUCT_TABLE(dest, X, size_var) _Generic((X), \
		struct record_t *: serialize_record_t , \
		default: memcpy \
	)(dest, X, size_var)

	#define DESERIALIZE_STRUCT_TABLE(dest, X, size_var) _Generic((X), \
		struct record_t *: deserialize_record_t , \
		default: memcpy \
	)(dest, X, size_var)


	#define MAKE_STRUCT_FIELD(type, ptr, _, name, ...) \
		type ptr name;
	
	#define MAKE_STRUCT_DEF(field_name, type, init_name, ...) \
		struct type { \
			field_name##_FIELDS(MAKE_STRUCT_FIELD) \
		} init_name ;

	#define MAKE_SERIALIZE_FIELD_NORMAL(var, size_var, ...) \
		size += sizeof(src->var); \
		if (size > max_size) { \
			fprintf(stderr, "%s: max_size(%ld) exceded\n", __func__, max_size); \
			return -1; \
		} \
		SERIALIZE_STRUCT_TABLE(dest, &(src->var), sizeof(src->var)); \
		dest += sizeof(src->var);

	#define MAKE_SERIALIZE_FIELD_PTR(var, size_var, ...) \
		size += src->size_var; \
		if (size > max_size) { \
			fprintf(stderr, "%s: max_size(%ld) exceded\n", __func__, max_size); \
			return -1; \
		} \
		SERIALIZE_STRUCT_TABLE(dest, src->var, src->size_var); \
		dest += src->size_var;
	
	#define MAKE_SERIALIZE_FIELD(type, ptr, is_p, var, size_var, ...) \
		MAKE_SERIALIZE_FIELD_##is_p(var, size_var)
	
	#define MAKE_SERIALIZE_FUNC(field_name, type, ...) \
		int serialize_##type (void* dest, struct type * src, size_t max_size) { \
			int size = 0; \
			field_name##_FIELDS(MAKE_SERIALIZE_FIELD) \
			return size; \
		}

	#define DECL_SERIALIZE_FUNC(field_name, type, ...) \
		int serialize_##type (void* dest, struct type * src, size_t max_size);






	#define MAKE_DESERIALIZE_FIELD_PTR(type, var, size_var, ...) \
		size += dest->size_var; \
		if (size > max_size) { \
			fprintf(stderr, "%s: max_size(%ld) exceded\n", __func__, max_size); \
			return -1; \
		} \
		dest->var = malloc(dest->size_var); \
		if (dest->var == NULL) { \
			perror("malloc"); \
			return -1; \
		} \
		DESERIALIZE_STRUCT_TABLE(dest->var, src, dest->size_var);  \
		src += dest->size_var;

	#define MAKE_DESERIALIZE_FIELD_NORMAL(type, var, size_var, ...) \
		size += sizeof(dest->var); \
		if (size > max_size) { \
			fprintf(stderr, "%s: max_size(%ld) exceded\n", __func__, max_size); \
			return -1; \
		} \
		DESERIALIZE_STRUCT_TABLE(&(dest->var), src, sizeof(dest->var)); \
		src += sizeof(dest->var);
		

	#define MAKE_DESERIALIZE_FIELD(type, ptr, is_p, var, size_var, ...) \
		MAKE_DESERIALIZE_FIELD_##is_p(type, var, size_var)
	
		
	#define MAKE_DESERIALIZE_FUNC(field_name, type, ...) \
		int deserialize_##type (struct type * dest, uint8_t * src, size_t max_size) { \
			int size = 0; \
			field_name##_FIELDS(MAKE_DESERIALIZE_FIELD) \
			return size; \
		}

	#define DECL_DESERIALIZE_FUNC(field_name, type, ...) \
		int deserialize_##type (struct type * dest, uint8_t * src, size_t useless);







	#define MAKE_FREE_FIELD_PTR(type, var, size_var, ...) \
		FREE_STRUCT_TABLE(ptr->var)
		
	// none because staticaly allocated variables are freed with parent struct
	#define MAKE_FREE_FIELD_NORMAL(type, var, size_var, ...)

	#define MAKE_FREE_FIELD(type, ptr, is_p, var, size_var, ...) \
		MAKE_FREE_FIELD_##is_p(type, var, size_var)
	
		
	#define MAKE_FREE_FUNC(field_name, type, ...) \
		int free_##type (struct type * ptr) { \
			field_name##_FIELDS(MAKE_FREE_FIELD) \
			return 0; \
		}

	#define DECL_FREE_FUNC(field_name, type, ...) \
		int free_##type (struct type * ptr);







	#define MAKE_SIZE_FIELD_PTR(type, var, size_var, ...) \
		size += ptr->size_var;
		
	// none because staticaly allocated variables are freed with parent struct
	#define MAKE_SIZE_FIELD_NORMAL(type, var, size_var, ...) \
		size += sizeof(ptr->var);


	#define MAKE_SIZE_FIELD(type, ptr, is_p, var, size_var, ...) \
		MAKE_SIZE_FIELD_##is_p(type, var, size_var)
	
		
	#define MAKE_SIZE_FUNC(field_name, type, ...) \
		size_t size_##type (struct type * ptr) { \
			size_t size=0; \
			field_name##_FIELDS(MAKE_SIZE_FIELD) \
			return size; \
		}

	#define DECL_SIZE_FUNC(field_name, type, ...) \
		size_t size_##type (struct type * ptr);






	STRUCTS(MAKE_STRUCT_DEF)
	STRUCTS(DECL_SERIALIZE_FUNC)
	STRUCTS(DECL_DESERIALIZE_FUNC)
	STRUCTS(DECL_FREE_FUNC)
	STRUCTS(DECL_SIZE_FUNC)

	STRUCTS(MAKE_SERIALIZE_FUNC)
	STRUCTS(MAKE_DESERIALIZE_FUNC)
	STRUCTS(MAKE_FREE_FUNC)
	STRUCTS(MAKE_SIZE_FUNC)
#endif
