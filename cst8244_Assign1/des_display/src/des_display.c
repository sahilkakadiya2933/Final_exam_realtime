#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "../../des_controller/src/des-mva.h"
#include <sys/neutrino.h>
#include <sys/dispatch.h> // Correct header for name_attach

int main(void) {
    int rcvid;
    Display display;

    // Phase I: Create channel for controller process to attach
    name_attach_t* attach = name_attach(NULL, "display", 0);

    // Check if name attach was successful
    if (attach == NULL) {
        perror("name_attach() failed\n");
        exit(EXIT_FAILURE);
    }

    // Print display PID
    printf("The display is running as PID: %d\n\n", getpid());

    // Phase II: Loop to display the correct messages corresponding to action event
    while (1) {
        rcvid = MsgReceive(attach->chid, &display, sizeof(Display), NULL);

        if (rcvid == -1) {
            perror("Message reception failed\n");
            exit(EXIT_FAILURE);
        }

        // Process the action message
        switch (display.indexOutMessage) {
            case OUT_LS_RS:
                printf("%s %d\n", outMessage[OUT_LS_RS], display.person.personId);
                break;
            case OUT_WS:
                printf("%s %d\n", outMessage[OUT_WS], display.person.weight);
                break;
            default:
                printf("%s\n", outMessage[display.indexOutMessage]);
                break;
        }

        // Check if message was replied/printed successfully
        if (MsgReply(rcvid, EOK, &display, sizeof(display)) == -1) {
            perror("Message reply failed\n");
            exit(EXIT_FAILURE);
        }

        // Exit if the action was EXIT
        if (display.indexOutMessage == OUT_EXIT)
            break;
    }

    // Phase III: Destroy channel
    name_detach(attach, 0);

    return EXIT_SUCCESS;
}
