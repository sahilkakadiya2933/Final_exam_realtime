#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <fstream>

using namespace std;

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

int main(int argc, char *argv[]) {
	/* Configure myController as a server; register the device within the namespace */
	name_attach_t *attach;
	if ((attach = name_attach(NULL, "mydevice", 0)) == NULL) { //call name_attach() and register the device name: “mydevice” exit FAILURE if name_attach() failed
		perror("Failed to create channel");
		return EXIT_FAILURE;
	}

	/* Upon startup, myController is to read the status of the device:	*/
	FILE * fd;
	fd = fopen("dev/local/mydevice", "r"); //call fopen() to open the device: /dev/local/mydevice

	if (fd != NULL) {
		string status = "";
		string value = "";
		fscanf(fd, "%s", status); //scan the status and value from the file
		fscanf(fd, "%s", value);
		//OK HERE
		printf("Status: %s\n", status); //not printing ??
		printf("Value: %s\n", value); //not printing ??

		if (status == "status") { //if the status is “status” then printf %s, value
			printf("Status: %s\n", value);
		}
		if (value == "closed") { //then if the value is “closed” then name_detach and exit with SUCCESS
			name_detach(attach, 0);
			return EXIT_SUCCESS;
		}
		fclose(fd); //close the device by its fd
	}

	while (true) {
		my_message_t msg;
		string status;
		string value;
		int rcvid;
		string tempFdHolder;

		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == 0) {
			if (rcvid == MY_PULSE_CODE) {
				printf("Small Integer: %d\n", msg.pulse.value.sival_int);
				ifstream file("/dev/local/mydevice");
				string all;
				status = "";
				value = "";
				getline(file, all);

				bool word1 = true;
				for (char& c : all) {
					if (c == '\n') {
						continue;
					}
					if (c == ' ') {
						word1 = false;
						continue;
					}
					if (word1) {
						status += c;
					} else {
						value += c;
					}
				}
				if (status == "status") {
					cout << "Status: " << value << endl;
					if (value == "closed") {
						name_detach(attach, 0);
						file.close();
						return EXIT_SUCCESS;
					}
				}
				fclose(fd);
			}
		} else if (rcvid != 0) {
			printf("ERROR receiving msg!");
			return EXIT_FAILURE;
		}
	}
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}

