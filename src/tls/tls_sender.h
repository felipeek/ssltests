#ifndef RAWHTTPS_TLS_SENDER_H
#define RAWHTTPS_TLS_SENDER_H
#include "protocol.h"
#include "../util.h"
// send to the client a new HANDSHAKE packet, with message type SERVER_HELLO
int rawhttps_tls_sender_handshake_server_hello_message_send(const rawhttps_connection_state* server_cs, int connected_socket, unsigned short selected_cipher_suite,
	unsigned char* random_number, dynamic_buffer* handshake_messages);
// send to the client a new HANDSHAKE packet, with message type SERVER_CERTIFICATE
// for now, this function receives a single certificate!
// @todo: support a chain of certificates
int rawhttps_tls_sender_handshake_server_certificate_message_send(const rawhttps_connection_state* server_cs, int connected_socket,
	unsigned char* certificate, int certificate_size, dynamic_buffer* handshake_messages);
// send to the client a new HANDSHAKE packet, with message type SERVER_HELLO_DONE
int rawhttps_tls_sender_handshake_server_hello_done_message_send(const rawhttps_connection_state* server_cs, int connected_socket,
	dynamic_buffer* handshake_messages);
// send to the client a new HANDSHAKE packet, with message type FINISHED
int rawhttps_tls_sender_handshake_finished_message_send(const rawhttps_connection_state* server_cs, int connected_socket, unsigned char verify_data[12]);
// send to the client a new CHANGE_CIPHER_SPEC message
int rawhttps_tls_sender_change_cipher_spec_send(const rawhttps_connection_state* server_cs, int connected_socket);
// send to the client a new APPLICATION_DATA message
int rawhttps_tls_sender_application_data_send(const rawhttps_connection_state* server_cs, int connected_socket,
	unsigned char* content, long long content_length);

#endif