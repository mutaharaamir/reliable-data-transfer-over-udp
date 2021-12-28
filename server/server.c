#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <binn.h>

#define ACK_LENGTH 100
#define MESSAGE_LENGTH 500

struct packet
{
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
    // char ack[ACK_LENGTH] = "Next seq num1";
    char clientMessage[MESSAGE_LENGTH];

    FILE *outputFileptr;

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

            outputFileptr = fopen("output_file.mp4", "wb");

            int lastSequenceNumber = -1;

            while (1)
            {
                int reply = recvfrom(sock, clientMessage, MESSAGE_LENGTH, 0, (struct sockaddr *)&client, &clientLength);
                pkt.data = binn_object_str(clientMessage, "pay_load");
                pkt.sequenceNumber = binn_object_int32(clientMessage, "seq_number");

                binn *seqnum;
                seqnum = binn_object();

                if (reply >= 0)
                {
                    printf("Received message from %s and port %i\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    //
                    int response;
                    if (pkt.sequenceNumber == lastSequenceNumber + 1)
                    {
                        printf("Sequence number: %d\n", pkt.sequenceNumber);
                        fwrite(pkt.data, 1, MESSAGE_LENGTH, outputFileptr);
                        binn_object_set_int32(seqnum, "seq_number", pkt.sequenceNumber + 1);
                        lastSequenceNumber++;
                        response = sendto(sock, binn_ptr(seqnum), binn_size(seqnum), 0, (struct sockaddr *)&client, sizeof(client));
                    }
                    else
                    {
                        binn_object_set_int32(seqnum, "seq_number", lastSequenceNumber + 1);
                        response = sendto(sock, binn_ptr(seqnum), binn_size(seqnum), 0, (struct sockaddr *)&client, sizeof(client));
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