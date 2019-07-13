#include "record.h"
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include "aes_cbc.h"

#define RECORD_PARSER_CHUNK_SIZE 1024
#define RECORD_BUFFER_INITIAL_SIZE 1024

#define LITTLE_ENDIAN_16(x) (((unsigned short)(x)[1]) | ((unsigned short)(x)[0] << 8))
#define LITTLE_ENDIAN_24(x) (((unsigned int)(x)[2]) | ((unsigned int)(x)[1] << 8) | ((unsigned int)(x)[0] << 16))

// creates rawhttps_parser_state
int rawhttps_record_buffer_create(rawhttps_record_buffer* record_buffer)
{
	record_buffer->buffer = malloc(sizeof(char) * RECORD_BUFFER_INITIAL_SIZE);
	if (!record_buffer->buffer) return -1;
	record_buffer->buffer_size = RECORD_BUFFER_INITIAL_SIZE;
	record_buffer->buffer_end = 0;
	record_buffer->buffer_position_fetch = 0;
	record_buffer->buffer_position_get = 0;
	return 0;
}

// destroys rawhttps_parser_buffer
void rawhttps_record_buffer_destroy(rawhttps_record_buffer* record_buffer)
{
	free(record_buffer->buffer);
}

// clear the phb buffer.
// data which was already used via 'get' functions will be released and the pointers will be adjusted
static void rawhttps_record_buffer_clear(rawhttps_record_buffer* record_buffer)
{
	memmove(record_buffer->buffer, record_buffer->buffer + record_buffer->buffer_position_get,
		record_buffer->buffer_end - record_buffer->buffer_position_get);
	record_buffer->buffer_end -= record_buffer->buffer_position_get;
	record_buffer->buffer_position_fetch -= record_buffer->buffer_position_get;
	record_buffer->buffer_position_get = 0;
}

// fetches the next chunk of tcp data and stores in the phb buffer
static long long record_fetch_next_tcp_chunk(rawhttps_record_buffer* record_buffer, int connected_socket)
{
	long long size_needed = record_buffer->buffer_end + RECORD_PARSER_CHUNK_SIZE;
	if (size_needed > record_buffer->buffer_size)
	{
		record_buffer->buffer = realloc(record_buffer->buffer, size_needed);
		record_buffer->buffer_size = size_needed;
	}

	long long size_read;
	if ((size_read = read(connected_socket, record_buffer->buffer + record_buffer->buffer_end, RECORD_PARSER_CHUNK_SIZE)) < 0)
		return -1;
	if (size_read == 0)
		return -1;
	record_buffer->buffer_end += size_read;

	return size_read;
}

// guarantees that the next 'num' bytes are available in the phb buffer.
static int record_guarantee_next_bytes(rawhttps_record_buffer* record_buffer, int connected_socket, unsigned char** ptr, long long num)
{
	while (record_buffer->buffer_position_fetch + num > record_buffer->buffer_end)
		if (record_fetch_next_tcp_chunk(record_buffer, connected_socket) == -1)
			return -1;

	record_buffer->buffer_position_fetch += num;
	*ptr = record_buffer->buffer + record_buffer->buffer_position_fetch - num;
	return 0;
}

// guarantees that the next record packet is available as a whole in the phb buffer.
static int record_guarantee_record(rawhttps_record_buffer* record_buffer, int connected_socket)
{
	unsigned char* ptr;

	// fetch record header.
	// the fourth/fifth bytes are the length
	if (record_guarantee_next_bytes(record_buffer, connected_socket, &ptr, 5))
		return -1;

	unsigned short record_length = LITTLE_ENDIAN_16(ptr + 3);

	// get record
	if (record_guarantee_next_bytes(record_buffer, connected_socket, &ptr, record_length))
		return -1;

	return 0;
}

// gets next 'num' bytes from phb buffer.
// this function basically increments the internal buffer_position_get pointer and returns a pointer to the data via 'ptr'
// if the data was not fetched previously by the 'fetch' functions, an error is returned.
static int record_get_next_bytes(rawhttps_record_buffer* record_buffer, long long num, unsigned char** ptr)
{
	if (record_buffer->buffer_position_get + num > record_buffer->buffer_position_fetch)
		return -1;

	record_buffer->buffer_position_get += num;
	*ptr = record_buffer->buffer + record_buffer->buffer_position_get - num;
	return 0;
}

static int record_data_decrypt(const rawhttps_connection_state* client_connection_state, unsigned char* record_data,
	unsigned short record_data_length, unsigned char* result)
{
	switch (client_connection_state->security_parameters.cipher)
	{
		case CIPHER_STREAM: {
			memcpy(result, record_data, record_data_length);
			return record_data_length;
		} break;
		case CIPHER_BLOCK: {
			unsigned char record_iv_length = client_connection_state->security_parameters.record_iv_length;
			unsigned short record_data_without_iv_length = record_data_length - (unsigned char)record_iv_length;
			unsigned char* record_iv = record_data;
			unsigned char* record_data_without_iv = record_data + record_iv_length;
			int block_count = (int)record_data_without_iv_length / client_connection_state->security_parameters.block_length;
			// @TODO: here we should depend on bulk algorithm
			aes_128_cbc_decrypt(record_data_without_iv, client_connection_state->cipher_state.enc_key, record_iv, block_count, result);
			unsigned char padding_length = result[record_data_without_iv_length - 1];
			return record_data_without_iv_length - client_connection_state->security_parameters.mac_length - padding_length - 1;
		} break;
		case CIPHER_AEAD: {
			printf("Cipher type not supported\n");
			return -1;
		} break;
	}

	return -1;
}

// gets the data of the next record packet and stores in the received buffer. The type is also returned via 'type'
long long rawhttps_record_get_data(rawhttps_record_buffer* record_buffer, int connected_socket,
	unsigned char* data, protocol_type* type, const rawhttps_connection_state* client_connection_state)
{
	unsigned char* ptr;

	if (record_guarantee_record(record_buffer, connected_socket))
		return -1;

	if (record_get_next_bytes(record_buffer, 5, &ptr))
		return -1;
	unsigned short record_length = LITTLE_ENDIAN_16(ptr + 3);
	*type = *ptr;
	assert(record_length <= RECORD_PROTOCOL_TLS_CIPHER_TEXT_MAX_SIZE);
	if (record_get_next_bytes(record_buffer, record_length, &ptr))
		return -1;

	long long decrypted_record_data_length = record_data_decrypt(client_connection_state, ptr, record_length, data);
	assert(decrypted_record_data_length <= RECORD_PROTOCOL_TLS_PLAIN_TEXT_MAX_SIZE);
	rawhttps_record_buffer_clear(record_buffer);
	return decrypted_record_data_length;
}