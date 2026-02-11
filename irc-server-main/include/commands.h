#ifndef COMMANDS_H
#define COMMANDS_H

#include "channel.h"

#define RPL_WELCOME 001
#define RPL_LISTSTART 321
#define RPL_LIST 322
#define RPL_LISTEND 323
#define RPL_NAMREPLY 353
#define RPL_ENDOFNAMES 366
#define RPL_WHOREPLY 352
#define RPL_ENDOFWHO 315

#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NONICKNAMEGIVEN 431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE 433
#define ERR_NEEDMOREPARAMS 461

typedef struct {
    char* command;
    char* params[15];
    int param_count;
} parsed_command_t;

// Command handlers
void cmd_nick(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_user(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_join(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_part(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_privmsg(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_list(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_names(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_who(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_ping(server_state_t* state, client_t* client, parsed_command_t* cmd);
void cmd_quit(server_state_t* state, client_t* client, parsed_command_t* cmd);

// Parser and dispatcher
int parse_irc_command(char* line, parsed_command_t* cmd);
void handle_command(server_state_t* state, client_t* client, parsed_command_t* cmd);

// Helpers
void send_raw(int sockfd, const char* msg);
void send_numeric_reply(int sockfd, int code, const char* nick, const char* msg);
void send_names_reply(client_t* client, channel_t* channel);
int is_member(channel_t* channel, client_t* client);

#endif
