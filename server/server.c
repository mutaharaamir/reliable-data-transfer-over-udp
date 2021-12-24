#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <binn.h>

#define ACK_LENGTH 100
#define MESSAGE_LENGTH 1000

struct packet {
    int sequenceNumber;
    char *data;
};

int main()
{

    struct packet pkt;

    int sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int clientLength = sizeof(client);
    char ack[ACK_LENGTH] = "Next seq num 1";
    char clientMessage[MESSAGE_LENGTH];

    //cleaning buffers.
    memset(clientMessage, '\0', MESSAGE_LENGTH);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock >= 0)
    {
        printf("Socket created successfully.\n");
        server.sin_family = AF_INET;
        server.sin_port = htons(2000);
        server.sin_addr.s_addr = inet_addr("127.0.0.1");

        //Binding to port.
        int bindValue = bind(sock, (struct sockaddr *)&server, sizeof(server));
        if (bindValue >= 0)
        {
            printf("Binded to port successfully.\n");
            int reply = recvfrom(sock, clientMessage, MESSAGE_LENGTH, 0, (struct sockaddr *)&client, &clientLength);

            pkt.data = binn_object_str(clientMessage, "pay_load");

            if (reply >= 0)
            {
                printf("Received message from %s and port %i\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                printf("Message: %s\n", pkt.data);
            
                int response = sendto(sock, ack, ACK_LENGTH, 0, (struct sockaddr *)&client, sizeof(client));

                if (response >= 0)
                {
                    printf("Successfully responded.\n");
                }
                else
                {
                    printf("Could not respond.\n");
                    close(sock);
                    return -1;
                }
            }
            else
            {
                printf("Could not receive a reply.\n");
                close(sock);
                return -1;
            }
        }
        else
        {
            printf("Could not bind to port.\n");
            close(sock);
            return -1;
        }
    }
    else
    {
        printf("Error creating socket\n");
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}