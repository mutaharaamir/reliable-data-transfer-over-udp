#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
    int serverLength = sizeof(server);

    FILE *fptr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock >= 0)
    {
        printf("Socket created successfully.\n");
        server.sin_family = AF_INET;
        server.sin_port = htons(2000);
        server.sin_addr.s_addr = inet_addr("127.0.0.1");

        int seqNum = 0;

        fptr = fopen("sample_video.mp4", "rb");

        if (fptr == NULL)
        {
            printf("Error opening file.\n");
            close(sock);
            return -1;
        }

        while (1)
        {
            struct packet pkt;
            pkt.seqNum = seqNum;

            memset(pkt.data, '\0', MESSAGE_LENGTH);

            size_t num_read = fread(pkt.data, 1, MESSAGE_LENGTH, fptr);

            struct acknowledgement ack;

            //EOF.
            if (num_read == 0)
            {
                pkt.seqNum = -5;
                int msg = sendto(sock, &pkt, sizeof(struct packet), 0, (struct sockaddr *)&server, sizeof(server));

                if (msg >= 0)
                {
                    printf("Send complete. Fin signal sent.\n");
                }
                else
                {
                    printf("Error sending fin signal.\n");
                }

                break;
            }

            int msg = sendto(sock, &pkt, sizeof(struct packet), 0, (struct sockaddr *)&server, sizeof(server));

            if (msg >= 0)
            {
                printf("Sent message successfully.\n");

                int reply = recvfrom(sock, &ack, sizeof(struct acknowledgement), 0, (struct sockaddr *)&server, &serverLength);

                if (reply >= 0)
                {

                    //If the same packet is requested again.
                    if (ack.ackNum == pkt.seqNum)
                    {   
                        //Moving fptr read bytes back.
                        fseek(fptr, -num_read, SEEK_CUR);
                    }

                    printf("Next seq num req: %d\n", ack.ackNum);
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

            seqNum++;
        } 

        fclose(fptr);
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