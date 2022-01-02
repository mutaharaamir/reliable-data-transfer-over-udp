#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MESSAGE_LENGTH 500

struct packet
{
    int seqNum;
    char data[MESSAGE_LENGTH];
};

struct acknowledgement
{
    int ackNum;
};

int main()
{

    int sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int clientLength = sizeof(client);

    FILE *outputFileptr;

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

            outputFileptr = fopen("output.mp4", "wb");

            int lastSeqNumReceived = -1;

            while (1)
            {

                struct packet pkt;
                memset(pkt.data, '\0', MESSAGE_LENGTH);

                int reply = recvfrom(sock, &pkt, sizeof(struct packet), 0, (struct sockaddr *)&client, &clientLength);

                if (reply >= 0)
                {

                    int response;

                    int fin = pkt.seqNum;
                    if (fin == -5)
                    {
                        fclose(outputFileptr);
                        printf("File write complete.\n");
                        break;
                    }

                    if (pkt.seqNum != lastSeqNumReceived + 1)
                    {
                        struct acknowledgement ack;
                        ack.ackNum = lastSeqNumReceived + 1;

                        response = sendto(sock, &ack, sizeof(struct acknowledgement), 0, (struct sockaddr *)&client, sizeof(client));
                    }
                    else
                    {

                        printf("Received seq num: %d\n", pkt.seqNum);

                        fwrite(pkt.data, 1, MESSAGE_LENGTH, outputFileptr);

                        struct acknowledgement ack;
                        ack.ackNum = pkt.seqNum + 1;

                        response = sendto(sock, &ack, sizeof(struct acknowledgement), 0, (struct sockaddr *)&client, sizeof(client));
                        lastSeqNumReceived++;
                    }

                    if (response >= 0)
                    {
                        printf("Successfully responded.\nListening..\n");
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