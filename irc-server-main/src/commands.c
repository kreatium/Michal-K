#include "../include/commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

void send_raw(int sockfd, const char* msg) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s\r\n", msg);
    write(sockfd, buf, strlen(buf));
}

void send_numeric_reply(int sockfd, int code, const char* nick, const char* msg) {
    char buf[1024];
    snprintf(buf, sizeof(buf), ":irc.server %03d %s %s", code, nick, msg);
    send_raw(sockfd, buf);
}

void send_names_reply(client_t* client, channel_t* channel) {
    char names_buf[1024];
    int pos = 0;
    for (int i = 0; i < channel->member_count; i++) {
        pos += snprintf(names_buf + pos, sizeof(names_buf) - pos, "%s%s", i ? " " : "", channel->members[i]->nickname);
    }

    char names_reply[1024];
    snprintf(names_reply, sizeof(names_reply), ":irc.server 353 %s = %s :%s", client->nickname, channel->name, names_buf);
    send_raw(client->sockfd, names_reply);

    snprintf(names_reply, sizeof(names_reply), ":irc.server 366 %s %s :Koniec listy NAMES", client->nickname, channel->name);
    send_raw(client->sockfd, names_reply);
}

int is_member(channel_t* channel, client_t* client) {
    for (int i = 0; i < channel->member_count; i++) {
        if (channel->members[i] == client) {
            return 1;
        }
    }
    return 0;
}

int parse_irc_command(char* line, parsed_command_t* cmd) {
    if (!line || !cmd || strlen(line) == 0) {
        return 0;
    }

    cmd->param_count = 0;
    cmd->command = NULL;

    char* ptr = line;

    while (*ptr == ' ') ptr++;

    if (*ptr == '\0') return 0;

    // Parse command
    cmd->command = ptr;
    while (*ptr && *ptr != ' ') ptr++;
    if (*ptr) *ptr++ = '\0';

    // Parse parameters
    while (*ptr && cmd->param_count < 15) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '\0') break;

        if (*ptr == ':') {
            ptr++;
            cmd->params[cmd->param_count++] = ptr;
            break;
        }

        cmd->params[cmd->param_count++] = ptr;
        while (*ptr && *ptr != ' ') ptr++;
        if (*ptr) *ptr++ = '\0';
    }

    return 1;
}

typedef void (*cmd_handler_t)(server_state_t*, client_t*, parsed_command_t*);

static struct {
    const char* name;
    cmd_handler_t handler;
    int require_reg;
} commands[] = {
    {"NICK", cmd_nick, 0},
    {"USER", cmd_user, 0},
    {"JOIN", cmd_join, 1},
    {"PART", cmd_part, 1},
    {"PRIVMSG", cmd_privmsg, 1},
    {"LIST", cmd_list, 1},
    {"NAMES", cmd_names, 1},
    {"WHO", cmd_who, 1},
    {"PING", cmd_ping, 1},
    {"QUIT", cmd_quit, 1},
    {NULL, NULL, 0}
};

void handle_command(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (!state || !client || !cmd || !cmd->command) {
        return;
    }

    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcasecmp(cmd->command, commands[i].name) == 0) {
            if (commands[i].require_reg && !client->registered) {
                return;
            }
            commands[i].handler(state, client, cmd);
            return;
        }
    }
}

// Command handlers
void cmd_nick(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 1) {
        send_numeric_reply(client->sockfd, ERR_NONICKNAMEGIVEN, "*", ":Nie podano nicka");
        return;
    }

    const char* nick = cmd->params[0];

    if (strlen(nick) == 0 || strlen(nick) > 9) {
        send_numeric_reply(client->sockfd, ERR_ERRONEUSNICKNAME, "*", ":Błędny nick");
        return;
    }

    for (size_t i = 0; i < strlen(nick); i++) {
        char c = nick[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            send_numeric_reply(client->sockfd, ERR_ERRONEUSNICKNAME, "*", ":Błędny nick");
            return;
        }
    }

    client_t* existing = find_client_by_nick(state, nick);
    if (existing && existing != client) {
        send_numeric_reply(client->sockfd, ERR_NICKNAMEINUSE, "*", ":Nick jest już używany");
        return;
    }

    strncpy(client->nickname, nick, 9);
    client->nickname[9] = '\0';

    if (client->nickname[0] != '\0' && client->username[0] != '\0' && !client->registered) {
        client->registered = 1;
        send_numeric_reply(client->sockfd, RPL_WELCOME, client->nickname, ":Witamy w IRC");
    }
}

void cmd_user(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 4) {
        send_numeric_reply(client->sockfd, ERR_NEEDMOREPARAMS, "*", "USER :Za mało parametrów");
        return;
    }

    strncpy(client->username, cmd->params[0], 9);
    client->username[9] = '\0';

    strncpy(client->realname, cmd->params[3], 63);
    client->realname[63] = '\0';

    if (client->nickname[0] != '\0' && client->username[0] != '\0' && !client->registered) {
        client->registered = 1;
        send_numeric_reply(client->sockfd, RPL_WELCOME, client->nickname, ":Witamy w IRC");
    }
}

void cmd_join(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 1) {
        send_numeric_reply(client->sockfd, ERR_NEEDMOREPARAMS, client->nickname, "JOIN :Za mało parametrów");
        return;
    }

    const char* channel_name = cmd->params[0];

    if (channel_name[0] != '#') {
        send_numeric_reply(client->sockfd, ERR_NOSUCHCHANNEL, client->nickname, ":Nie ma takiego kanału");
        return;
    }

    channel_t* channel = find_channel(state, channel_name);
    if (!channel) {
        channel = create_channel(state, channel_name);
        if (!channel) {
            send_raw(client->sockfd, "ERROR :Osiągnięto limit kanałów serwera");
            return;
        }
    }

    int result = add_client_to_channel(channel, client);
    if (result == -1) {
        send_raw(client->sockfd, "ERROR :Kanał jest pełny");
        return;
    }
    if (result == 0) {
        return;
    }

    char join_msg[512];
    snprintf(join_msg, sizeof(join_msg), ":%s!%s@localhost JOIN %s", client->nickname, client->username, channel_name);
    send_raw(client->sockfd, join_msg);

    for (int i = 0; i < channel->member_count; i++) {
        if (channel->members[i] != client) {
            send_raw(channel->members[i]->sockfd, join_msg);
        }
    }

    send_names_reply(client, channel);
}

void cmd_part(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 1) {
        send_numeric_reply(client->sockfd, ERR_NEEDMOREPARAMS, client->nickname, "PART :Za mało parametrów");
        return;
    }

    const char* channel_name = cmd->params[0];

    channel_t* channel = find_channel(state, channel_name);
    if (!channel) {
        send_numeric_reply(client->sockfd, ERR_NOSUCHCHANNEL, client->nickname, ":Nie ma takiego kanału");
        return;
    }

    char part_msg[512];
    snprintf(part_msg, sizeof(part_msg), ":%s!%s@localhost PART %s", client->nickname, client->username, channel_name);

    for (int i = 0; i < channel->member_count; i++) {
        send_raw(channel->members[i]->sockfd, part_msg);
    }

    remove_client_from_channel(channel, client);
}

void cmd_privmsg(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 2) {
        send_numeric_reply(client->sockfd, ERR_NEEDMOREPARAMS, client->nickname, "PRIVMSG :Za mało parametrów");
        return;
    }

    const char* target = cmd->params[0];
    const char* message = cmd->params[1];

    // Channel message
    if (target[0] == '#') {
        channel_t* channel = find_channel(state, target);
        if (!channel) {
            send_numeric_reply(client->sockfd, ERR_NOSUCHCHANNEL, client->nickname, ":Nie ma takiego kanału");
            return;
        }

        if (!is_member(channel, client)) {
            send_numeric_reply(client->sockfd, ERR_CANNOTSENDTOCHAN, client->nickname, ":Nie można wysłać do kanału");
            return;
        }

        char msg_buf[1024];
        snprintf(msg_buf, sizeof(msg_buf), ":%s!%s@localhost PRIVMSG %s :%s", client->nickname, client->username, target, message);

        for (int i = 0; i < channel->member_count; i++) {
            if (channel->members[i] != client) {
                send_raw(channel->members[i]->sockfd, msg_buf);
            }
        }
    }
    // User message
    else {
        client_t* target_client = find_client_by_nick(state, target);
        if (!target_client) {
            send_numeric_reply(client->sockfd, ERR_NOSUCHNICK, client->nickname, ":Nie ma takiego nicka");
            return;
        }

        char msg_buf[1024];
        snprintf(msg_buf, sizeof(msg_buf), ":%s!%s@localhost PRIVMSG %s :%s", client->nickname, client->username, target, message);
        send_raw(target_client->sockfd, msg_buf);
    }
}

void cmd_list(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    send_numeric_reply(client->sockfd, RPL_LISTSTART, client->nickname, "Kanał :Użytkownicy  Nazwa");

    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (state->channels[i].active) {
            char list_reply[512];
            snprintf(list_reply, sizeof(list_reply), ":irc.server 322 %s %s %d :", client->nickname, state->channels[i].name, state->channels[i].member_count);
            send_raw(client->sockfd, list_reply);
        }
    }

    send_numeric_reply(client->sockfd, RPL_LISTEND, client->nickname, ":Koniec LISTY");
}

void cmd_names(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 1) {
        send_numeric_reply(client->sockfd, ERR_NEEDMOREPARAMS, client->nickname, "NAMES :Za mało parametrów");
        return;
    }

    const char* channel_name = cmd->params[0];

    channel_t* channel = find_channel(state, channel_name);
    if (!channel) {
        send_numeric_reply(client->sockfd, ERR_NOSUCHCHANNEL, client->nickname, ":Nie ma takiego kanału");
        return;
    }

    send_names_reply(client, channel);
}

void cmd_who(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    if (cmd->param_count < 1) {
        send_numeric_reply(client->sockfd, ERR_NEEDMOREPARAMS, client->nickname, "WHO :Za mało parametrów");
        return;
    }

    const char* target = cmd->params[0];

    if (target[0] == '#') {
        channel_t* channel = find_channel(state, target);
        if (!channel) {
            send_numeric_reply(client->sockfd, ERR_NOSUCHCHANNEL, client->nickname, ":Nie ma takiego kanału");
            return;
        }

        for (int i = 0; i < channel->member_count; i++) {
            client_t* member = channel->members[i];
            char who_reply[512];
            snprintf(who_reply, sizeof(who_reply),
                     ":irc.server 352 %s %s %s localhost irc.server %s H :0 %s",
                     client->nickname, target, member->username, member->nickname, member->realname);
            send_raw(client->sockfd, who_reply);
        }

        char end_reply[512];
        snprintf(end_reply, sizeof(end_reply), ":irc.server 315 %s %s :Koniec listy WHO", client->nickname, target);
        send_raw(client->sockfd, end_reply);
    }
}

void cmd_ping(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    char pong_msg[512];
    if (cmd->param_count > 0) {
        snprintf(pong_msg, sizeof(pong_msg), "PONG :%s", cmd->params[0]);
    } else {
        snprintf(pong_msg, sizeof(pong_msg), "PONG :irc.server");
    }
    send_raw(client->sockfd, pong_msg);
}

void cmd_quit(server_state_t* state, client_t* client, parsed_command_t* cmd) {
    char quit_msg[512];
    if (cmd->param_count > 0) {
        snprintf(quit_msg, sizeof(quit_msg), ":%s!%s@localhost QUIT :%s", client->nickname, client->username, cmd->params[0]);
    } else {
        snprintf(quit_msg, sizeof(quit_msg), ":%s!%s@localhost QUIT :Client quit", client->nickname, client->username);
    }

    client_t* sent[MAX_CLIENTS];
    int sent_count = 0;

    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (state->channels[i].active && is_member(&state->channels[i], client)) {
            for (int j = 0; j < state->channels[i].member_count; j++) {
                client_t* member = state->channels[i].members[j];
                if (member == client) continue;

                int already_sent = 0;
                for (int k = 0; k < sent_count; k++) {
                    if (sent[k] == member) {
                        already_sent = 1;
                        break;
                    }
                }

                if (!already_sent) {
                    send_raw(member->sockfd, quit_msg);
                    sent[sent_count++] = member;
                }
            }
        }
    }

    remove_client(state, client);

    close(client->sockfd);
}
