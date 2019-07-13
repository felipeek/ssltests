#ifndef RAWHTTPS_PARSER_H
#define RAWHTTPS_PARSER_H
#include "protocol.h"
#include "util.h"
#include "record.h"
typedef struct {
	unsigned char* buffer;
	long long buffer_size;
	long long buffer_end;

	long long buffer_position_get;
	long long buffer_position_fetch;
} rawhttps_message_buffer;

typedef struct {
	rawhttps_record_buffer record_buffer;
	rawhttps_message_buffer message_buffer;
	protocol_type type;
} rawhttps_parser_state;

int rawhttps_parser_state_create(rawhttps_parser_state* ps);
int rawhttps_parser_state_destroy(rawhttps_parser_state* ps);
int rawhttps_parser_parse_ssl_packet(const rawhttps_connection_state* client_connection_state, tls_packet* packet,
	rawhttps_parser_state* ps, int connected_socket, dynamic_buffer* handshake_messages);

#endif