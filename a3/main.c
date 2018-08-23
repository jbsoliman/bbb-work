#include "audioMixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "udpclient.h"
#include "beatPlayer.h"
#include "zenController.h"


int main() {

    printf("Starting the program\n");
    printf("\n\nTo connect to UDP server, open new terminal with the following command:\n");
    printf("netcat -u 192.168.7.2 12345\n\n");
    UDP_init();
    Zencape_init();
    Zencape_checkState();
    Beatplayer_init();
    Beatplayer_stop();
    UDP_stop();

	return 0;
}
