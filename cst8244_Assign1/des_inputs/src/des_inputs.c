#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include "../../des_controller/src/des-mva.h"

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

int main(void) {
    int coid;
    char userInput[25];
    Person person;

    // Phase I: Open connection to the controller using the name service
    coid = name_open("controller", 0);
    if (coid == -1) {
        perror("Could not connect to controller\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to controller\n");

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
            perror("Failed to send message\n");
            exit(EXIT_FAILURE);
        }

        if (strcmp(userInput, inMessage[EXIT]) == 0) {
            break;
        }
    }

    // Phase III: Close connection to the controller
    name_close(coid);
    return EXIT_SUCCESS;
}
