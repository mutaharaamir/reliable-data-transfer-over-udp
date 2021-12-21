#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define ACK_LENGTH 100
#define MESSAGE_LENGTH 1024

int main()
{

    int sock;
    struct sockaddr_in server;
    int serverLength = sizeof(server);
    char ack[ACK_LENGTH];
    char clientMessage[MESSAGE_LENGTH];

    //cleaning buffers.
    memset(ack, '\0', ACK_LENGTH);
    memset(clientMessage, '\0', MESSAGE_LENGTH);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock >= 0)
    {
        printf("Socket created successfully.\n");
        server.sin_family = AF_INET;
        server.sin_port = htons(2000);
        server.sin_addr.s_addr = inet_addr("127.0.0.1");

        printf("Enter Message: ");
        fgets(clientMessage, MESSAGE_LENGTH, stdin);

        int msg = sendto(sock, clientMessage, MESSAGE_LENGTH, 0, (struct sockaddr *)&server, sizeof(server));

        if (msg >= 0)
        {
            printf("Sent message successfully.\n");

            int reply = recvfrom(sock, ack, sizeof(ack), 0, (struct sockaddr *)&server, &serverLength);

            if (reply >= 0)
            {
                printf("ACK: %s\n", ack);
            }
            else
            {
                printf("Error receiving message.\n");
                close(sock);
                return -1;
            }
        }
        else
        {
            printf("Error sending message.\n");
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