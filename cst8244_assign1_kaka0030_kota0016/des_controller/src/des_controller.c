#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "des-mva.h"
#include <fcntl.h>

// Function prototypes
void *startIdleState();
void *doorScanState();
void *guardFirstDoorUnlockState();
void *firstDoorOpenState();
void *weightScanState();
void *firstDoorCloseState();
void *guardFirstDoorLockState();
void *guardSecondDoorUnlockState();
void *secondDoorOpenState();
void *secondDoorCloseState();
void *guardSecondDoorLockState();
void *exitState();

void checkExitState();

Display display;
Person person;
NextState nextState = startIdleState;
Direction direction = DEFAULT;
int controllerCoid;

int main(int argc, char* argv[]) {
    int rcvid;
    int controllerChid;
    pid_t displayPid;

    if (argc != 2) {
        fprintf(stderr, "Missing Display's PID\n");
        exit(EXIT_FAILURE);
    }
    displayPid = atoi(argv[1]);

    controllerChid = ChannelCreate(0);
    if (controllerChid == -1) {
        perror("Failed to create channel\n");
        exit(EXIT_FAILURE);
    }

    controllerCoid = ConnectAttach(ND_LOCAL_NODE, displayPid, 1, _NTO_SIDE_CHANNEL, 0);
    printf("The controller is running as PID: %d\nWaiting for Person...\n\n", getpid());

    while (1) {
        if (nextState == startIdleState) {
            nextState = (NextState)(*nextState)();
        }

        rcvid = MsgReceive(controllerChid, &person, sizeof(Person), NULL);

        if (MsgReply(rcvid, EOK, &person, sizeof(Person)) == -1) {
            perror("Controller failed to reply input\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(person.event, inMessage[EXIT]) == 0) {
            (*exitState)();
            break;
        }

        nextState = (NextState)(*nextState)();
    }

    return EXIT_SUCCESS;
}

void checkExitState() {
    if (strcmp(person.event, inMessage[EXIT]) == 0) {
        (*exitState)();
    }
}

void *startIdleState() {
    direction = DEFAULT;
    person.weight = -1;
    person.personId = -99;
    display.person = person;

    return doorScanState;
}

void *doorScanState() {
    if ((strcmp(person.event, inMessage[RS]) == 0 || strcmp(person.event, inMessage[LS]) == 0) && direction == DEFAULT) {
        direction = strcmp(person.event, inMessage[RS]) == 0 ? RIGHT : LEFT;
        display.person = person;
        display.indexOutMessage = OUT_LS_RS;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (doorScanState)\n");
            exit(EXIT_FAILURE);
        }

        return guardFirstDoorUnlockState;
    }

    return doorScanState;
}

void *guardFirstDoorUnlockState() {
    if ((strcmp(person.event, inMessage[GLU]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[GRU]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_GLU : OUT_GRU;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (guardFirstDoorUnlockState)\n");
            exit(EXIT_FAILURE);
        }

        return firstDoorOpenState;
    }

    return guardFirstDoorUnlockState;
}

void *firstDoorOpenState() {
    if ((strcmp(person.event, inMessage[LO]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[RO]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_LO : OUT_RO;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (firstDoorOpenState)\n");
            exit(EXIT_FAILURE);
        }

        return weightScanState;
    }

    return firstDoorOpenState;
}

void *weightScanState() {
    if (strcmp(person.event, inMessage[WS]) == 0) {
        display.person = person;
        display.indexOutMessage = OUT_WS;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (weightScanState)\n");
            exit(EXIT_FAILURE);
        }

        return firstDoorCloseState;
    }

    return weightScanState;
}

void *firstDoorCloseState() {
    if ((strcmp(person.event, inMessage[LC]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[RC]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_LC : OUT_RC;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (firstDoorCloseState)\n");
            exit(EXIT_FAILURE);
        }

        return guardFirstDoorLockState;
    }

    return firstDoorCloseState;
}

void *guardFirstDoorLockState() {
    if ((strcmp(person.event, inMessage[GLL]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[GRL]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_GLL : OUT_GRL;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (guardFirstDoorLockState)\n");
            exit(EXIT_FAILURE);
        }

        return guardSecondDoorUnlockState;
    }

    return guardFirstDoorLockState;
}

void *guardSecondDoorUnlockState() {
    if ((strcmp(person.event, inMessage[GRU]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[GLU]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_GRU : OUT_GLU;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (guardSecondDoorUnlockState)\n");
            exit(EXIT_FAILURE);
        }

        return secondDoorOpenState;
    }

    return guardSecondDoorUnlockState;
}

void *secondDoorOpenState() {
    if ((strcmp(person.event, inMessage[RO]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[LO]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_RO : OUT_LO;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (secondDoorOpenState)\n");
            exit(EXIT_FAILURE);
        }

        return secondDoorCloseState;
    }

    return secondDoorOpenState;
}

void *secondDoorCloseState() {
    if ((strcmp(person.event, inMessage[RC]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[LC]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_RC : OUT_LC;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (secondDoorCloseState)\n");
            exit(EXIT_FAILURE);
        }

        return guardSecondDoorLockState;
    }

    return secondDoorCloseState;
}

void *guardSecondDoorLockState() {
    if ((strcmp(person.event, inMessage[GRL]) == 0 && direction == LEFT) || (strcmp(person.event, inMessage[GLL]) == 0 && direction == RIGHT)) {
        display.person = person;
        display.indexOutMessage = direction == LEFT ? OUT_GRL : OUT_GLL;

        if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
            perror("Controller failed to send message (guardSecondDoorLockState)\n");
            exit(EXIT_FAILURE);
        }

        return startIdleState;
    }

    return guardSecondDoorLockState;
}

void *exitState() {
    if (strcmp(person.event, inMessage[EXIT]) == 0) {
        display.indexOutMessage = OUT_EXIT;
    }

    if (MsgSend(controllerCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1L) {
        perror("Controller failed to send message (exitState)\n");
        exit(EXIT_FAILURE);
    }

    printf("Exiting controller\n");

    return exitState;
}
