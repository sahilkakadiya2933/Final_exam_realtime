#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include "../../calc_server/src/calc_message.h"

void print_usage_and_exit() {
    printf("Usage: ProcessID integer character integer\n");
    exit(EXIT_FAILURE);
}

void check_arguments_range(double left, double right) {
    if (left < INT_MIN || right < INT_MIN || left > INT_MAX || right > INT_MAX) {
        printf("Numerical arguments should be Integer.\n");
        exit(EXIT_SUCCESS);
    }
}

int establish_connection(pid_t serverpid) {
    int coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0);
    if (coid < 0) {
        printf("Couldn't Connect to server\n");
        exit(EXIT_FAILURE);
    }
    return coid;
}

void send_message(int coid, client_send_t *sendMessage, server_response_t *receiveMessage) {
    if (MsgSend(coid, sendMessage, sizeof(*sendMessage), receiveMessage, sizeof(*receiveMessage)) < 0) {
        printf("Error sending Message\n");
        exit(EXIT_FAILURE);
    }
}

void handle_response(client_send_t *sendMessage, server_response_t *receiveMessage) {
    switch (receiveMessage->statusCode) {
        case SRVR_OK:
            printf("%d %c %d = %.2f\n", sendMessage->left_hand, sendMessage->operator, sendMessage->right_hand, receiveMessage->answer);
            break;
        case SRVR_INVALID_OPERATOR:
            printf("Invalid Operator\n");
            break;
        case SRVR_OVERFLOW:
            printf("Overflow\n");
            break;
        case SRVR_UNDEFINED:
            printf("Undefined\n");
            break;
        default:
            printf("Unknown error\n");
    }
    if (receiveMessage->statusCode != SRVR_OK) {
        printf("%s\n", receiveMessage->errorMsg);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage_and_exit();
    }

    double left = atof(argv[2]);
    double right = atof(argv[4]);
    check_arguments_range(left, right);

    pid_t serverpid = atoi(argv[1]);
    int coid = establish_connection(serverpid);

    client_send_t sendMessage;
    sendMessage.left_hand = (int)left;
    sendMessage.operator = *argv[3];
    sendMessage.right_hand = (int)right;

    server_response_t receiveMessage;
    send_message(coid, &sendMessage, &receiveMessage);
    handle_response(&sendMessage, &receiveMessage);

    ConnectDetach(coid);
    return EXIT_SUCCESS;
}
