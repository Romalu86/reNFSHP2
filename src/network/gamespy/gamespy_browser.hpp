#pragma once
#include "gamespy_http_runtime.hpp"
namespace nfs::network::session::pending::browser {
using Compare = int(__cdecl *)(const void *, const void *);
using Hash = Dword(__cdecl *)(const void *, Dword);
using Destroy = void(__cdecl *)(void *);
struct HashTable {
    void **buckets;
    int bucket_count;
    Destroy destroy;
    Hash hash;
    Compare compare;
};
struct Cipher {
    Dword table[256], first_stack[16], second_stack[16], branch_stack[16];
    int depth;
    Dword first, second, branch, seed;
    Byte buffer[64], *cursor;
};
struct Server {
    Dword address;
    std::uint16_t port;
    std::int16_t ping;
    Dword query_type;
    HashTable *properties, *strings;
};
struct Query {
    SOCKET socket;
    Server *server;
    Dword started;
    sockaddr endpoint;
};
struct Browser;
using Callback = void(__cdecl *)(Browser *, Dword, Dword, Server *, int);
struct Browser {
    Dword phase;
    void *servers;
    Query *queries;
    char game[32], key[32], query_game[32];
    int query_count, next_server;
    Dword cancel_requested;
    Callback callback;
    Dword context;
    const char *error;
    Dword reserved_84;
    SOCKET socket;
    Dword broadcast_started, query_type;
    HashTable *strings;
    int previous_count;
    Cipher cipher;
    Dword row_phase;
};
static_assert(sizeof(HashTable) == 20 && sizeof(Server) == 20 && sizeof(Query) == 28);
static_assert(sizeof(Cipher) == 1304 && offsetof(Cipher, cursor) == 1300);
static_assert(sizeof(Browser) == 1464 && offsetof(Browser, cipher) == 156 &&
              offsetof(Browser, row_phase) == 1460);
void insert_array(void *, const void *, int);
void remove_array_without_destroy(void *, int);
void replace_array(void *, const void *, int);
int search_array(void *, const void *, Compare, int, Dword);
HashTable *create_hash(int, int, int, Hash, Compare, Destroy);
void free_hash(HashTable *);
void enter_hash(HashTable *, const void *);
void *find_hash(HashTable *, const void *);
Dword __cdecl hash_interned_string(const void *, Dword);
Dword __cdecl hash_property(const void *, Dword);
int __cdecl compare_strings(const void *, const void *);
int __cdecl compare_property_keys(const void *, const void *);
void __cdecl free_interned_string(void *);
void __cdecl free_server_slot(void *);
char *duplicate_string(const char *);
char *next_token(char *, char);
char *intern_string(Server *, const char *);
Dword property_is_retained(const char *);
Server *create_server(Dword, std::uint16_t, Dword, HashTable *);
Server *parse_server_row(char **, int, char *, Dword, HashTable *);
void parse_server_properties(Server *, char *);
int server_ping(Server *);
char *server_address_text(Server *);
Dword server_address(Server *);
std::uint16_t server_port(Server *);
const char *server_text(Server *, const char *, const char *);
int server_integer(Server *, const char *, int);
Browser *create(const char *, const char *, const char *, int, Callback, Dword, Dword);
void destroy(Browser *);
Dword refresh_master(Browser *, Dword, const char *, Dword);
Dword refresh_lan(Browser *, Dword, std::uint16_t, int, int);
Dword refresh_server(Browser *, const char *, int, Dword, Dword);
void change_phase(Browser *, Dword);
Server *append_server(Browser *, Dword, std::uint16_t, Dword);
int find_server(Browser *, Dword, int, int);
Dword pump_lan(Browser *);
Dword pump_master(Browser *);
Dword pump_queries(Browser *);
Dword pump(Browser *);
int consume_server_row(Browser *, char *, int);
Dword cancel(Browser *);
Dword clear(Browser *);
Dword phase(Browser *);
void encode_validation(const Byte *, int, char *);
void encrypt_validation(const Byte *, int, Byte *, int);
Dword seed_cipher(Cipher *, Dword, Dword);
void generate_cipher_words(Cipher *, Dword *, int);
void initialize_cipher(const Byte *, Dword, Cipher *);
void decrypt(Cipher *, Byte *, int);
Dword ticks();
void sleep(int);
extern Byte master_buffer[2048];
extern Dword master_bytes;
extern char *master_columns[20];
extern int master_column_count;
Dword pump_master_stream(Browser *, bool);
bool allocate_query_sockets(Browser *);
void pump_synchronously(Browser *);
} // namespace nfs::network::session::pending::browser
namespace nfs::network::authentication::pending::unresolved {
void initialize_sockets(); // Shares its reference count with frontend socket release.
                            
}
