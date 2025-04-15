#include <stdint.h>


typedef struct
{
    uint16_t RCODE : 4;
    uint16_t Z : 3;
    uint16_t RA : 1;
    uint16_t RD : 1;
    uint16_t TC : 1;
    uint16_t AA : 1;
    uint16_t OPCODE : 4;
    uint16_t QR : 1;
} dns_flags_t;

typedef struct
{
    uint16_t id;
    union
    {
        dns_flags_t flags_bitfields;
        uint16_t flags_u16;
    };
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header_t;

typedef struct {
    
    uint16_t type;
    uint16_t class;
}dns_question_t;
// typedef struct {

// }answer;
// typedef struct {

// }authority;
// typedef struct {

// }additional;
// typedef struct {
//     dns_header_t header;
//     dns_question_t question;

// }message;