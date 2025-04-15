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

		printf("\nHeader buffer:\n");

		uint8_t domain_name_length = 12;
		size_t domain_name_length_size = sizeof(domain_name_length);
		char domain_name[] = "codecrafters";
		size_t domain_name_size = sizeof(domain_name) - 1;
		uint8_t domain_length = 2;
		size_t domain_length_size = sizeof(domain_length);
		char domain[] = "io";
		size_t domain_size = sizeof(domain);
		uint16_t question_type = htons(1);
		size_t question_type_size = sizeof(question_type);
		uint16_t question_class = htons(1);
		size_t question_class_size = sizeof(question_class);

		size_t question_size = domain_name_length_size + domain_name_size + domain_length_size + domain_size + question_type_size + question_class_size;

		header.qdcount = htons(header.qdcount + 1);

		// memset(buf, 0, buf_size);

		printf("\nQuestion buffer:\n");

		// for(size_t i = 0; i < sizeof(buf); ++i) {
		// 	printf("%x ", buf[i]);
		// };

		// Create an empty response
		unsigned char response[header_size + question_size];

		memcpy(response, &header, header_size);
		memcpy(response + header_size, &domain_name_length, domain_name_length_size);
		memcpy(response + header_size + domain_name_length_size, domain_name, domain_name_size);
		memcpy(response + header_size + domain_name_length_size + domain_name_size, &domain_length, domain_length_size);
		memcpy(response + header_size + domain_name_length_size + domain_name_size + domain_length_size, domain, domain_size);
		memcpy(response + header_size + domain_name_length_size + domain_name_size + domain_length_size + domain_size, &question_type, question_type_size);
		memcpy(response + header_size + domain_name_length_size + domain_name_size + domain_length_size + domain_size + question_type_size, &question_class, question_class_size);

		printf("\nResponse buffer:\n");

		for (size_t i = 0; i < sizeof(response); ++i)
		{
			printf("%x ", response[i]);
		};

		// Send response
		if (sendto(udpSocket, response, sizeof(response), 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1)
		{
			perror("Failed to send response");
		}
	}

	close(udpSocket);

	return 0;
}
