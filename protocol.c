
#include "protocol.h"
#include <math.h>

struct format_t global_format;
// wrappers for implementing TLS
int send_data(int fd, void* data, size_t size) {
	return write(fd, data, size);
}
int recv_data(int fd, void* data, size_t size) {
	size_t size_old = size;
	int out;
	while (size != 0) {
		out=read(fd, data, size);
		if(out < 0) {
			perror("recv_data");
			return size_old - size;
		}
		size-=out;
		data+=out;
	}
	return size_old;
}

int generate_global_format() {
	uint32_t num_of_structs = 0;
	COUNT(num_of_structs, STRUCTS);
	global_format.num_of_structs = num_of_structs;
	global_format.struct_info = (struct struct_info_t*) malloc(sizeof(struct struct_info_t) * global_format.num_of_structs);
	if (global_format.struct_info == NULL) {
		perror("malloc");
		return -1;
	}
	int iterator = 0;
	int field_iterator = 0;
	STRUCTS(MAKE_STRUCT_DEFS);
	return 0;
}

int send_format_to_server(int fd) {
	uint32_t num_structs = htonl(global_format.num_of_structs);
	if (send_data(fd, &num_structs, 4 )!= 4) {
		printf("Error while sending num of structs to server\n");
		return -1;
	}
	for(int i=0; i<global_format.num_of_structs; i++) {
		char terminator = '\0';
		uint32_t size = 0;
		size+=sizeof(global_format.struct_info[i].essential);
		size+=strlen(global_format.struct_info[i].identifier)+1;
		for(int j=0;j<global_format.struct_info[i].num_of_fields; j++) {
			size+=sizeof(global_format.struct_info[i].field_info[j].essential);
			size+=strlen(global_format.struct_info[i].field_info[j].identifier)+1;
			size+=sizeof(global_format.struct_info[i].field_info[j].size);
			size+=sizeof(global_format.struct_info[i].field_info[j].collabsible_to);
		}
		uint32_t size_net = htonl(size);
		if (send_data(fd, &size_net, sizeof(size_net)) != 4) {
			printf("Error while sending packet size to server\n");
			return -2;
		}

		if (send_data(fd, &global_format.struct_info[i].essential, 1) != 1) {
			printf("Error while sending struct essential to server\n");
			return -3;
		}
		if (send_data(fd, global_format.struct_info[i].identifier, strlen(global_format.struct_info[i].identifier)) != strlen(global_format.struct_info[i].identifier)) {
			printf("Error while sending struct name to server\n");
			return -4;
		}
		if (send_data(fd, &terminator, sizeof(terminator)) != 1) {
			printf("Error while sending null terminator to server\n");
			return -5;
		}
		for(int j=0;j<global_format.struct_info[i].num_of_fields; j++) {
			if (send_data(fd, &global_format.struct_info[i].field_info[j].essential, 1) != 1) {
				printf("Error while sending field essential to server\n");
				return -6;
			}
			if (send_data(fd, &global_format.struct_info[i].field_info[j].size, 1) != 1) {
				printf("Error while sending field size to server\n");
				return -7;
			}
			if (send_data(fd, &global_format.struct_info[i].field_info[j].collabsible_to, 1) != 1) {
				printf("Error while sending field collabsible_to to server\n");
				return -8;
			}
			printf("essential=%d ; size=%d ; collabsible_to=%d\n", global_format.struct_info[i].field_info[j].essential, global_format.struct_info[i].field_info[j].size, global_format.struct_info[i].field_info[j].collabsible_to );
			if (send_data(fd, global_format.struct_info[i].field_info[j].identifier, strlen(global_format.struct_info[i].field_info[j].identifier)) != strlen(global_format.struct_info[i].field_info[j].identifier)) {
				printf("Error while sending field name to server\n");
				return -9;
			}
			if (send_data(fd, &terminator, sizeof(terminator)) != 1) {
				printf("Error while sending null terminator to server\n");
				return -5;
			}
		}
	}
	return 0;
}
struct format_mask_t* malloc_mask() {
	struct format_mask_t* mask = (struct format_mask_t*) malloc(sizeof(struct format_mask_t));
	if (mask == NULL) {
		perror("malloc");
		return NULL;
	}
	mask->num_of_structs = global_format.num_of_structs;
	mask->struct_mask = (struct struct_mask_t*) malloc(sizeof(struct struct_mask_t)*mask->num_of_structs);
	if (mask->struct_mask == NULL) {
		perror("malloc");
		free(mask);
		return NULL;
	}
	for(int i=0;i<mask->num_of_structs;i++){
		mask->struct_mask[i].num_of_fields = global_format.struct_info[i].num_of_fields;
		mask->struct_mask[i].field_mask = (uint8_t*) malloc(mask->struct_mask[i].num_of_fields);
		memset(mask->struct_mask[i].field_mask, 0, mask->struct_mask[i].num_of_fields);
		if (mask->struct_mask[i].field_mask == NULL) {
			perror("malloc");
			for (int j=i;j>0;j--) {
				free(mask->struct_mask[j].field_mask);
			}
			free(mask);
			return NULL;
		}
	}
	return mask;
}
void free_mask(struct format_mask_t* mask) {
	for (int i=0;i<mask->num_of_structs;i++) {
		free(mask->struct_mask[i].field_mask);
	}
	free(mask->struct_mask);
	free(mask);
}
int32_t generate_mask_from_client(int fd, struct format_mask_t* mask) {
	int32_t total_num_of_struct=0;

	uint32_t num_of_structs;
	int err=0;
	if ((err = recv_data(fd, &num_of_structs, 4)) != 4) {
		printf("Error reciving num_of_supported_structs: %d\n", err);
		return  -6;
	}
	num_of_structs = ntohl(num_of_structs);

	for (int i=0;i<num_of_structs;i++) {
		uint32_t size=0;
		if ((err = recv_data(fd, &size, 4)) != 4) {
			printf("Error reciving packet_size: %d\n", err);
			return -6;
		}
		size = ntohl(size);
		uint8_t* data = (uint8_t*) malloc(size);
		if (data == NULL) {
			perror("malloc");
			return -5;

		}
		if ((err =recv_data(fd, data, size)) != size) {
			printf("Error reciving data: %d\n", err);
			free(data);
			return -6;
		}
		uint32_t iter=0;

		uint8_t is_essential=0;
		READ_SAFE_FORMAT(is_essential, iter, size, data, "struct")
		
		size_t struct_name_len = strnlen((const char*) data+iter, size-iter);
		if (size-iter == struct_name_len) {
			free(data);
			printf("Struct not null terminated\n");
			return -4;
		}
		

		for (int j=0;j<global_format.num_of_structs;j++)  {
			if (strcmp(global_format.struct_info[j].identifier, (char*) data+iter) == 0) {
				total_num_of_struct++;
				iter+=struct_name_len+1;
				while (size-iter != 0) {
					uint8_t essential=0;
					READ_SAFE_FORMAT(essential, iter, size, data, "field")
					uint8_t max_size=0;
					READ_SAFE_FORMAT(max_size, iter, size, data,"field")
					uint8_t collabsible_to=0;
					READ_SAFE_FORMAT(collabsible_to, iter, size, data,"field")
					printf("essential=%d ; size=%d ; collabsible_to=%d\n", essential, max_size, collabsible_to);

					size_t field_name_len = strnlen((const char*) data+iter, size-iter);
					if (size-iter == field_name_len) {
						free(data);
						printf("Field not null terminated\n");
						return -4;
					}
					for (int k=0;k<global_format.struct_info[j].num_of_fields;k++) {
						if (strcmp(global_format.struct_info[j].field_info[k].identifier, (char*)data+iter) == 0) {
							uint8_t max_collabsible = (collabsible_to >= global_format.struct_info[j].field_info[k].collabsible_to ? collabsible_to : global_format.struct_info[j].field_info[k].collabsible_to);
							uint8_t min_size = (max_size >= global_format.struct_info[j].field_info[k].size ? global_format.struct_info[j].field_info[k].size : max_size);
							if(min_size >max_collabsible) {
								free(data);
								printf("Unable to provide field %s\n", global_format.struct_info[j].field_info[k].identifier);
								goto fail_field;
							}
							mask->struct_mask[j].field_mask[k] = min_size;
							iter+=field_name_len+1;
							goto skip;
						}
					}
				fail_field:
					iter+= field_name_len+1;
					if (essential == 1) {
						free(data);
						printf("Essential field not supported by server\n");
						return -2;
					}
				skip:
					continue;
				}
				goto skip_outer;
			}
		} 
		if (is_essential == 1) {
			free(data);
			printf("Essential struct not supported by server\n");
			return -2;
		}
	skip_outer:
		free(data);
		continue;
	}
	// clean up the mask and check for server requirements
	for (int i=0;i<global_format.num_of_structs;i++) {
		
		uint8_t struct_exists=0;
		for(int j=0;global_format.struct_info[i].num_of_fields;j++) {
			if(mask->struct_mask[i].field_mask[j] != 0) {
				struct_exists = 1;

				
			}
			if (global_format.struct_info[i].field_info[j].essential == 1 && mask->struct_mask[i].field_mask[j] == 0) {
				printf("Essential field not supported by client\n");
				return -1; 
			} 
		}
		if (struct_exists == 0) {
			free(mask->struct_mask[i].field_mask);
			mask->struct_mask[i].field_mask = NULL;
			if (global_format.struct_info[i].essential == 1) {
				printf("Essential struct not supported by client\n");
				return -1;
			}
		}
	}
	return total_num_of_struct;
}


int send_format_to_client(int fd, struct format_mask_t* mask, int32_t num_of_structs) {
	int32_t num_of_structs_net = htonl(num_of_structs);
	if (send_data(fd, &num_of_structs_net, 4) != 4) {
		perror("send_num_of_structs");
		return -6;
	}
	if (num_of_structs < 0) {
		return num_of_structs;
	}
	for (uint32_t i=0;i<global_format.num_of_structs;i++) {
		uint32_t size=0;
		if (mask->struct_mask[i].field_mask != NULL) {
			size+=strlen(global_format.struct_info[i].identifier); // struct_name
			size++; // null terminator
			size+=sizeof(uint32_t); // struct_identifier
			for(int j=0;j<global_format.struct_info[i].num_of_fields;i++) {
				if (mask->struct_mask[i].field_mask[j] != 0) {
					size++; // size
					size += strlen(global_format.struct_info[i].field_info[j].identifier);
					size++; // null terminator
				}
			}
			size = ntohl(size);
			if (send_data(fd, &size, 4) != 4) {
				perror("send_size");
				return -6;
			}
			if (send_data(fd, global_format.struct_info[i].identifier, strlen(global_format.struct_info[i].identifier)) !=strlen(global_format.struct_info[i].identifier)) {
				perror("send_struct_name");
				return -6;
			}
			char null_terminator = '\0';
			if (send_data(fd, &null_terminator, 1) != 1) {
				perror("send_null_terminator");
				return -6;
			}
			if (send_data(fd, &i, 4) != 4) {
				perror("send_struct_identifier");
				return -6;
			}
			for(int j=0;j<global_format.struct_info[i].num_of_fields;j++) {
				if (send_data(fd, mask->struct_mask[i].field_mask+j, 1) != 1) {
					perror("send_field_size");
					return -6;
				}
				if (send_data(fd, global_format.struct_info[i].field_info[j].identifier, strlen(global_format.struct_info[i].field_info[j].identifier)) == strlen(global_format.struct_info[i].field_info[j].identifier)) {
					perror("send_field_identifier");
					return -6;
				}
				if (send_data(fd, &null_terminator, 1) != 1) {
					perror("send_null_terminator");
					return -6;
				}
			}
		}
	}
	return 0;

}
// returns 2 int32_t arrays
int32_t** generate_format_from_server(int fd) {
	int32_t num_of_structs=0;
	if (recv_data(fd,&num_of_structs, 4) != 4) {
		perror("recv_num_of_structs");
		return NULL;
	}
	num_of_structs = ntohl(num_of_structs);
	if (num_of_structs < 0) {
		printf("error happened: %d\n", num_of_structs);
		return NULL;
	}






}

STRUCTS(MAKE_SUB_GET_FIELD)
void* get_field(enum structs struct_id, void* object, uint32_t field_id) {
	void* (*sub_funcs[])(void*, uint32_t) = {STRUCTS(MAKE_FUNCTION_POINTERS_SUB_GET)};
	if (struct_id >= global_format.num_of_structs) {
		printf("%s: struct_id too big\n", __func__);
		return NULL;
	}
	if (field_id >= global_format.struct_info[struct_id].num_of_fields) {
		printf("%s: field_id too big\n", __func__);
		return NULL;
	}
	if (object == NULL) {
		printf("%s: object is NULL\n", __func__);
		return NULL;
	}
	return (*sub_funcs)(object, field_id);
}
int send_struct(enum structs struct_id, void* src) {
	if (struct_id >= global_format.num_of_structs) {
		printf("%s: struct_id too big\n", __func__);
		return -5;
	}
	
}
