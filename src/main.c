#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "message.h"

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	int udpSocket, client_addr_len;
	struct sockaddr_in clientAddress;

	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(2053),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(udpSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int bytesRead;
	char buffer[512];
	socklen_t clientAddrLen = sizeof(clientAddress);

	while (1)
	{
		// Receive data
		bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddress, &clientAddrLen);
		if (bytesRead == -1)
		{
			perror("Error receiving data");
			break;
		}

		buffer[bytesRead] = '\0';
		printf("Received %d bytes: %s\n", bytesRead, buffer);
		for (size_t i = 0; i < sizeof(buffer); ++i)
		{
			printf("%x", buffer[i]);
		};

		// Header
		dns_header_t header;
		size_t header_size = sizeof(dns_header_t);
		header.id = htons(1234);
		header.flags_bitfields.QR = 1;
		header.flags_bitfields.OPCODE = 0;
		header.flags_bitfields.AA = 0;
		header.flags_bitfields.TC = 0;
		header.flags_bitfields.RD = 0;
		header.flags_bitfields.RA = 0;
		header.flags_bitfields.Z = 0;
		header.flags_bitfields.RCODE = 0;
		header.flags_u16 = htons(header.flags_u16);
		header.qdcount = 0;
		header.ancount = 0;
		header.nscount = 0;
		header.arcount = 0;

		// Question
		dns_question_t question;
		question.domain_name = "codecrafters";
		question.domain_name_length = strlen(question.domain_name);
		question.domain = "io";
		question.domain_length = strlen(question.domain);
		question.type = htons(1);
		question.class = htons(1);
		// size_t question_size = sizeof(question);

		header.qdcount = htons(header.qdcount + 1);

		// Anwser

		dns_answer_t answer;
		answer.domain_name = "codecrafters";
		answer.domain_name_length = strlen(answer.domain_name);
		answer.domain = "io";
		answer.domain_length = strlen(answer.domain);
		answer.type = htons(1);
		answer.class = htons(1);
		answer.rdata_length = htons(4);
		answer.ttl = htons(60);
		answer.ip_address_bitfields.s1 = 8;
		answer.ip_address_bitfields.s2 = 8;
		answer.ip_address_bitfields.s3 = 8;
		answer.ip_address_bitfields.s4 = 8;
		answer.rdata = htons(answer.ip_address_u32);

		header.ancount = htons(header.ancount + 1);

		// Header buffer:
		// 4 d2 80 0 0 1 0 0 0 0 0 0
		// Question buffer:
		// c 63 6f 64 65 63 72 61 66 74 65 72 73 2 69 6f 0 0 1 0 1
		// Response buffer:
		// 4 d2 80 0 0 1 0 0 0 0 0 0 c 63 6f 64 65 63 72 61 66 74 65 7 0 1 0 1

		// Create an empty response
		unsigned char response[1024];

		size_t offset = 0;

		// Header cpy
		memcpy(response, &header, header_size);
		offset += header_size;

		// Question cpy
		memcpy(&response[offset], &question.domain_name_length, sizeof(question.domain_name_length));
		offset += sizeof(question.domain_name_length);
		memcpy(&response[offset], question.domain_name, question.domain_name_length);
		offset += question.domain_name_length;
		memcpy(&response[offset], &question.domain_length, sizeof(question.domain_length));
		offset += sizeof(question.domain_length);
		memcpy(&response[offset], question.domain, question.domain_length);
		offset += question.domain_length;
		response[offset++] = '\0';
		memcpy(&response[offset], &question.type, sizeof(question.type));
		offset += sizeof(question.type);
		memcpy(&response[offset], &question.class, sizeof(question.class));
		offset += sizeof(question.class);

		// Answer cpy
		memcpy(&response[offset], &answer.domain_name_length, sizeof(answer.domain_name_length));
		offset += sizeof(answer.domain_name_length);
		memcpy(&response[offset], answer.domain_name, answer.domain_name_length);
		offset += answer.domain_name_length;
		memcpy(&response[offset], &answer.domain_length, sizeof(answer.domain_length));
		offset += sizeof(answer.domain_length);
		memcpy(&response[offset], answer.domain, answer.domain_length);
		offset += answer.domain_length;
		response[offset++] = '\0';
		memcpy(&response[offset], &answer.type, sizeof(answer.type));
		offset += sizeof(answer.type);
		memcpy(&response[offset], &answer.class, sizeof(answer.class));
		offset += sizeof(answer.class);

		memcpy(&response[offset], &answer.ttl, sizeof(answer.ttl));
		offset += sizeof(answer.ttl);
		memcpy(&response[offset], &answer.rdata_length, sizeof(answer.rdata_length));
		offset += sizeof(answer.rdata_length);
		memcpy(&response[offset], &answer.rdata, sizeof(answer.rdata));
		offset += sizeof(answer.rdata);

		printf("\nHeader buffer:\n");
		for (size_t i = 0; i < header_size; ++i)
		{
			printf("%x ", response[i]);
		};

		printf("\nQuestion buffer:\n");

		for (size_t i = header_size; i < offset; ++i)
		{
			printf("%x ", response[i]);
		};

		printf("\nResponse buffer:\n");

		for (size_t i = 0; i < offset; ++i)
		{
			printf("%x ", response[i]);
		};

		// Send response
		if (sendto(udpSocket, response, offset, 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1)
		{
			perror("Failed to send response");
		}
	}

	close(udpSocket);

	return 0;
}
