#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>

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

int sock;
int reply = -1;
struct sockaddr_in server;
int serverLength = sizeof(server);
struct acknowledgement ack;
pthread_t t1;
FILE *fptr;

void *thread1()
{
    reply = recvfrom(sock, &ack, sizeof(struct acknowledgement), 0, (struct sockaddr *)&server, &serverLength);
    printf("Ackkk************************************************\n");
}

void sendFile()
{

    int seqNum = 0;

    fptr = fopen("sample_video.mp4", "rb");

    if (fptr == NULL)
    {
        printf("Error opening file.\n");
        close(sock);
        return;
    }

    while (1)
    {
        struct packet pkt;
        pkt.seqNum = seqNum;

        memset(pkt.data, '\0', MESSAGE_LENGTH);

        size_t num_read = fread(pkt.data, 1, MESSAGE_LENGTH, fptr);

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
            printf("Sent message successfully. %d\n", pkt.seqNum);

            // int reply = recvfrom(sock, &ack, sizeof(struct acknowledgement), 0, (struct sockaddr *)&server, &serverLength);

            if (pthread_create(&t1, NULL, thread1, NULL) != 0)
            {
                printf("Error");
                return;
            }

            if (reply >= 0)
            {

                //If the same packet is requested again.
                if (ack.ackNum == pkt.seqNum)
                {
                    //Moving fptr read bytes back.
                    fseek(fptr, -num_read, SEEK_CUR);
                    seqNum--;
                }

                printf("Next seq num req: %d\n", ack.ackNum);
                reply = -1;
            }
            else
            {
                fseek(fptr, -num_read, SEEK_CUR);
                continue;
            }
        }
        else
        {
            printf("Error sending message.\n");
            close(sock);
            return;
        }

        seqNum++;
    }

    if (pthread_join(t1, NULL) != 0)
    {
        printf("Error");
        return;
    }
}

int main()
{

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock >= 0)
    {
        printf("Socket created successfully.\n");
        server.sin_family = AF_INET;
        server.sin_port = htons(2006);
        server.sin_addr.s_addr = inet_addr("127.0.0.1");

        // int seqNum = 0;

        sendFile();

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