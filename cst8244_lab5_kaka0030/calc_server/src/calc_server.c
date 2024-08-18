#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <math.h>
#include "calc_message.h"

#define SUCCESS 0
#define FAILURE -1

void print_server_pid() {
    printf("CalcServer PID: %d\n", getpid());
}

int create_channel() {
    int chid = ChannelCreate(0);
    if (chid < 0) {
        perror("Channel creation failed");
        exit(EXIT_FAILURE);
    }
    return chid;
}

void process_message(client_send_t *message, server_response_t *response) {
    int a = message->left_hand;
    int b = message->right_hand;
    double result = 0.0;
    response->statusCode = SRVR_OK;
    response->errorMsg[0] = '\0';

    switch (message->operator) {
        case '+':
            result = (double)a + b;
            break;
        case '-':
            result = (double)a - b;
            break;
        case 'x':
            result = (double)a * b;
            break;
        case '/':
            if (b == 0) {
                response->statusCode = SRVR_UNDEFINED;
                sprintf(response->errorMsg, "Undefined");
                return;
            } else {
                result = (double)a / b;
            }
            break;
        default:
            response->statusCode = SRVR_INVALID_OPERATOR;
            sprintf(response->errorMsg, "Invalid Operator");
            return;
    }

    if (result >= INT_MIN && result <= INT_MAX) {
        response->answer = result;
    } else {
        response->statusCode = SRVR_OVERFLOW;
        sprintf(response->errorMsg, "Overflow");
    }
}

void handle_requests(int chid) {
    client_send_t message;
    server_response_t response;
    int rcvid;

    while (1) {
        rcvid = MsgReceive(chid, &message, sizeof(message), NULL);
        if (rcvid < 0) {
            perror("MsgReceive failed");
            continue;
        }

        process_message(&message, &response);
        MsgReply(rcvid, EOK, &response, sizeof(response));
    }
}

int main() {
    int chid = create_channel();
    print_server_pid();
    handle_requests(chid);
    ChannelDestroy(chid);
    return EXIT_SUCCESS;
}
