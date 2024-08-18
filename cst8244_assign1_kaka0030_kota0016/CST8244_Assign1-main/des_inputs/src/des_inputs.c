#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "../../des_controller/src/des-mva.h"
#include <fcntl.h>

void getInput(char *prompt, char *input, size_t size) {
    printf("%s", prompt);
    scanf("%s", input);
}

void getPersonId(Person *person) {
    printf("Enter Person's ID:\n");
    scanf("%d", &person->personId);
    person->state = DOOR_SCAN_STATE;
}

void getPersonWeight(Person *person) {
    printf("Enter the Person's weight: \n");
    scanf("%d", &person->weight);
    person->state = WEIGHT_SCAN_STATE;
}

int main(int argc, char* argv[]) {
    int coid;
    char userInput[25];
    pid_t controllerPid;
    Person person;

    if (argc != 2) {
        fprintf(stderr, "Missing Controller's PID\n");
        exit(EXIT_FAILURE);
    }

    controllerPid = atoi(argv[1]);
    coid = ConnectAttach(ND_LOCAL_NODE, controllerPid, 1, _NTO_SIDE_CHANNEL, 0);

    if (coid == -1){
        fprintf(stderr, "Could not Connect Attach to ControllerPid\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        getInput("Enter the event type (ls = left scan, rs = right scan, ws = weight scale, lo = left open, ro = right open, lc = left closed, rc = right closed, gru = guard right unlock, grl = guard right lock, gll = guard left lock, glu = guard left unlock):\n", userInput, sizeof(userInput));

        if (strcmp(userInput, inMessage[RS]) == 0 || strcmp(userInput, inMessage[LS]) == 0) {
            getPersonId(&person);
            person.direction = strcmp(userInput, inMessage[RS]) == 0 ? RIGHT : LEFT;
        } else if (strcmp(userInput, inMessage[WS]) == 0) {
            getPersonWeight(&person);
        }

        strcpy(person.event, userInput);

        if (MsgSend(coid, &person, sizeof(Person), NULL, 0) == -1) {
            fprintf(stderr, "Failed to send message\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(userInput, inMessage[EXIT]) == 0) {
            break;
        }
    }

    ConnectDetach(coid);
    return EXIT_SUCCESS;
}
