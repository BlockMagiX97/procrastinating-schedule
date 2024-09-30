#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#ifndef PROTOCOL_H
#define PROTOCOL_H

struct field_info_t {
	uint8_t essential;
	uint8_t collabsible_to;
	const char* identifier;
	uint8_t size;
};
struct struct_info_t{
	uint32_t num_of_fields;
	uint8_t essential;
	const char* identifier;
	struct field_info_t* field_info;
};

struct format_t {
	uint32_t num_of_structs;
	struct struct_info_t* struct_info;
};

extern struct format_t global_format;

struct struct_mask_t {
	uint32_t num_of_fields;
	uint8_t* field_mask;
};
struct format_mask_t {
	uint32_t num_of_structs;
	struct struct_mask_t* struct_mask;
};

#define DEBUG_PRINT printf("%s:%s:%d\n", __FILE__, __func__, __LINE__);

#define READ_SAFE_FORMAT(var, iter, size, data, msg) \
		if (iter+sizeof(var) > size) { \
			printf(msg " " #var " not found\n"); \
			free(data); \
			return -4; \
		} \
		memcpy(&var, data+iter, sizeof(var)); \
		iter+=sizeof(var);
		


#define __COUNT(...) \
	i++;

#define COUNT(var, macro_list) \
	{ \
		uint64_t i=0; \
		macro_list(__COUNT); \
		var = i; \
	}

//	_F(MACRO_NAME, type_name, essential(1 means yes))
#define STRUCTS(_F, ...) \
	_F(STRUCT1, "STRUCT1", struct1, 1) \
	_F(STRUCT2, "STRUCT2", struct2, 0) \

//	_F(field_name, type, essential, max_size, min_size)
#define STRUCT1_FIELDS(_F, ...) \
	_F(x, int, "x", 0, sizeof(int), 2) \
	_F(y, char, "y", 1, 0, 0)

#define STRUCT2_FIELDS(_F, ...) \
	_F(z, char, "z", 1, 0, 0)

#define MAKE_STRUCT_FIELD_DEF(field_name, type, id, essential, max_size, min_size) \
	type field_name;
#define MAKE_STRUCT_DEF(macro, id, type_name, essential) \
	struct type_name { \
		macro##_FIELDS(MAKE_STRUCT_FIELD_DEF) \
	};

#define MAKE_FIELD_DEFS(field_name, type, id, essential_num, max_size, min_size) \
	global_format.struct_info[iterator].field_info[field_iterator].essential = essential_num; \
	global_format.struct_info[iterator].field_info[field_iterator].collabsible_to = min_size; \
	global_format.struct_info[iterator].field_info[field_iterator].size = max_size; \
	global_format.struct_info[iterator].field_info[field_iterator].identifier = id; \
	field_iterator++;

#define MAKE_STRUCT_DEFS(macro, id, type_name, essential_num) \
	COUNT(global_format.struct_info[iterator].num_of_fields, macro##_FIELDS); \
	global_format.struct_info[iterator].identifier = id; \
	global_format.struct_info[iterator].essential = essential_num; \
	global_format.struct_info[iterator].field_info = (struct field_info_t*) malloc(sizeof(struct field_info_t) * global_format.struct_info[iterator].num_of_fields); \
	if (global_format.struct_info[iterator].field_info == NULL) { \
		perror("malloc"); \
		return -1; \
	} \
	field_iterator = 0; \
	macro##_FIELDS(MAKE_FIELD_DEFS) \
	iterator++;

#define __MAKE_SUB_GET_FIELD(field_name, ...) \
	&(temp->field_name), 
	
#define MAKE_SUB_GET_FIELD(macro, id, type_name, essential) \
	void* _get_field_##macro (void* object, uint32_t field_id) { \
		struct type_name * temp = (struct type_name *) object; \
		void* array[] = {macro##_FIELDS(__MAKE_SUB_GET_FIELD)}; \
		return array[field_id]; \
	}

#define MAKE_FUNCTION_POINTERS_SUB_GET(macro, id, type_name, essential) \
	&_get_field_##macro, 
#define MAKE_ENUM_DEF(macro, id, typename, essential) \
	STRUCT_##macro, 

enum structs{STRUCTS(MAKE_ENUM_DEF)};



STRUCTS(MAKE_STRUCT_DEF)

int generate_global_format();
int32_t generate_mask_from_client(int fd, struct format_mask_t* mask);
struct format_mask_t* malloc_mask();
void free_mask(struct format_mask_t* mask);
int send_format_to_server(int fd);
int send_data(int fd, void* data, size_t size);
int recv_data(int fd, void* data, size_t size);
int send_format_to_client(int fd, struct format_mask_t* mask, int32_t num_of_structs);

void* get_field(uint32_t struct_id, void* object, uint32_t field_id);
#endif
