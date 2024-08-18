#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include "des-mva.h"

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
int displayCoid;

int main(void) {
    int rcvid;
    name_attach_t* attach;

    // Phase I: Connect to the display using the name service
    displayCoid = name_open("display", 0);
    if (displayCoid == -1) {
        perror("Failed to connect to display");
        exit(EXIT_FAILURE);
    }
    printf("Connected to display\n");

    // Phase I: Create a channel for the inputs process to attach
    attach = name_attach(NULL, "controller", 0);
    if (attach == NULL) {
        perror("Failed to create channel");
        exit(EXIT_FAILURE);
    }
    printf("Controller registered in namespace\n");

    printf("The controller is running as PID: %d\nWaiting for Person...\n\n", getpid());

    while (1) {
        if (nextState == startIdleState) {
            nextState = (NextState)(*nextState)();
        }

        // Phase II: Receive messages from the inputs process
        rcvid = MsgReceive(attach->chid, &person, sizeof(Person), NULL);
        if (rcvid == -1) {
            perror("Message reception failed");
            continue;
        }

        if (MsgReply(rcvid, EOK, &person, sizeof(Person)) == -1) {
            perror("Controller failed to reply input");
            exit(EXIT_FAILURE);
        }

        if (strcmp(person.event, inMessage[EXIT]) == 0) {
            (*exitState)();
            break;
        }

        nextState = (NextState)(*nextState)();
    }

    // Phase III: Clean up
    name_close(displayCoid);
    name_detach(attach, 0);

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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (doorScanState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (guardFirstDoorUnlockState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (firstDoorOpenState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (weightScanState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (firstDoorCloseState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (guardFirstDoorLockState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (guardSecondDoorUnlockState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (secondDoorOpenState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (secondDoorCloseState)");
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

        if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
            perror("Controller failed to send message (guardSecondDoorLockState)");
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

    if (MsgSend(displayCoid, &display, sizeof(Display), &display, sizeof(Display)) == -1) {
        perror("Controller failed to send message (exitState)");
        exit(EXIT_FAILURE);
    }

    printf("Exiting controller\n");

    return exitState;
}
