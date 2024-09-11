
#include "protocol.h"

// wrappers for implementing TLS
int send_data(int fd, void* data, size_t size) {
	return write(fd, data, size);
}
int recv_data(int fd, void* data, size_t size) {
	return read(fd, data, size);
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
	STRUCTS(MAKE_STRUCT_DEFS)
	return 0;
}

int send_format_to_server(int fd) {
	uint32_t num_structs = htonl(global_format.num_of_structs);
	if (send_data(fd, &num_structs, sizeof(num_structs)) != 4) {
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
		mask->struct_mask[i].num_of_fields = global_format.struct_info->num_of_fields;
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
int32_t generate_mask_from_client(int fd, struct format_mask_t* mask) {
	int32_t total_num_of_structs;

	uint32_t num_of_structs;
	if (recv_data(fd, &num_of_structs, 4) != 4) {
		perror("Error reciving num_of_supported_structs");
		return  -6;
	}
	num_of_structs = ntohl(num_of_structs);

	for (int i=0;i<num_of_structs;i++) {
		uint32_t size;
		if (recv_data(fd, &size, 4) != 4) {
			perror("Error reciving packet_size");
			return -6;
		}
		size = ntohl(size);
		uint8_t* data = (uint8_t*) malloc(size);
		if (data == NULL) {
			perror("malloc");
			return -5;

		}
		if (recv_data(fd, data, size) != size) {
			perror("Error reciving data");
			free(data);
			return -6;
		}
		uint32_t iter=0;

		uint8_t is_essential;
		READ_SAFE_FORMAT(is_essential, iter, size, data)
		
		size_t struct_name_len = strnlen((const char*) data+iter, size-iter);
		if (size-iter == struct_name_len) {
			free(data);
			printf("Struct not null terminated\n");
			return -4;
		}
		for (int j=0;j<global_format.num_of_structs;j++)  {
			if (strcmp(global_format.struct_info[j].identifier, (char*) data+iter) == 0) {
				total_num_of_structs++;
				iter+=struct_name_len+1;
				while (size-iter != 0) {
					uint8_t essential;
					READ_SAFE_FORMAT(essential, iter, size, data);
					uint8_t max_size;
					READ_SAFE_FORMAT(max_size, iter, size, data);
					uint8_t collabsible_to;
					READ_SAFE_FORMAT(collabsible_to, iter, size, data);
					size_t field_name_len = strnlen((const char*) data+iter, size-iter);
					if (size-iter == field_name_len) {
						free(data);
						printf("Field not null terminated\n");
						return -4;
					}
					for (int k;k<global_format.struct_info[j].num_of_fields;k++) {
						if (strcmp(global_format.struct_info[j].field_info[k].identifier, (char*)data+iter) == 0) {
							mask->struct_mask[j].field_mask[k] = max_size;
							goto skip;
						}
					}
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
		continue;
	}
	return total_num_of_structs;
}


int send_format_to_client(int fd) {

}
