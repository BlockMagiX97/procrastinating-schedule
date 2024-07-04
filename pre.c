// this file includes auto generation for struct related functions (write, read, free)
// free_(type) frees all pointer references, BUT THE STRUCT ITSELF MUST BE FREED !!!!!!!!!!!!!!
// free does NOT support pointers to pointers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

	#define WRITE_STRUCT_TABLE(X, size_var) _Generic((X), \
		struct record_t *: write_record_t , \
		default: write \
	)(fd, X, size_var)

	#define READ_STRUCT_TABLE(X, size_var) _Generic((X), \
		struct record_t *: read_record_t , \
		default: read \
	)(fd, X, size_var)

	#define MAKE_STRUCT_FIELD(type, ptr, _, name, ...) \
		type ptr name;
	
	#define MAKE_STRUCT_DEF(field_name, type, init_name, ...) \
		struct type { \
			field_name##_FIELDS(MAKE_STRUCT_FIELD) \
		} init_name ;

	#define MAKE_WRITE_FIELD_PTR(type, var, size_var, ...) \
		if (WRITE_STRUCT_TABLE(src->var, src->size_var * sizeof(type)) != src->size_var * sizeof(type)) { \
			perror("write_ptr"); \
			return -1; \
		}

	#define MAKE_WRITE_FIELD_NORMAL(type, var, size_var, ...) \
		if (WRITE_STRUCT_TABLE(&(src->var), sizeof(type)) != sizeof(type)) { \
			perror("write"); \
			return -1; \
		}

	#define MAKE_WRITE_FIELD(type, ptr, is_p, var, size_var, ...) \
		MAKE_WRITE_FIELD_##is_p(type, var, size_var)
	
		
	#define MAKE_WRITE_FUNC(field_name, type, ...) \
		int write_##type (int fd, struct type * src, size_t useless) { \
			field_name##_FIELDS(MAKE_WRITE_FIELD) \
			return 0; \
		}
	#define DECL_WRITE_FUNC(field_name, type, ...) \
		int write_##type (int fd, struct type * src, size_t useless);

	#define MAKE_READ_FIELD_PTR(type, var, size_var, ...) \
		dest->var = malloc(dest->size_var * sizeof(type)); \
		if (dest->var == NULL) { \
			perror("malloc"); \
			return -1; \
		} \
		if (READ_STRUCT_TABLE(dest->var, dest->size_var * sizeof(type)) != dest->size_var * sizeof(type)) { \
			perror("read_ptr"); \
			return -1; \
		}

	#define MAKE_READ_FIELD_NORMAL(type, var, size_var, ...) \
		if (READ_STRUCT_TABLE(&(dest->var), sizeof(type)) != sizeof(type)) { \
			perror("read"); \
			return -1; \
		}

	#define MAKE_READ_FIELD(type, ptr, is_p, var, size_var, ...) \
		MAKE_READ_FIELD_##is_p(type, var, size_var)
	
		
	#define MAKE_READ_FUNC(field_name, type, ...) \
		int read_##type (int fd, struct type * dest, size_t useless) { \
			field_name##_FIELDS(MAKE_READ_FIELD) \
			return 0; \
		}

	#define DECL_READ_FUNC(field_name, type, ...) \
		int read_##type (int fd, struct type * dest, size_t useless);

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

	STRUCTS(MAKE_STRUCT_DEF)
	STRUCTS(DECL_WRITE_FUNC)
	STRUCTS(DECL_READ_FUNC)
	STRUCTS(DECL_FREE_FUNC)

	STRUCTS(MAKE_WRITE_FUNC)
	STRUCTS(MAKE_READ_FUNC)
	STRUCTS(MAKE_FREE_FUNC)
#endif
