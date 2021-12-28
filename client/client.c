#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <binn.h>

#define ACK_LENGTH 100
#define MESSAGE_LENGTH 496

int main()
{
    int sock;
    struct sockaddr_in server;
    int serverLength = sizeof(server);
    char ack[ACK_LENGTH];
    char clientMessage[MESSAGE_LENGTH];

    FILE *fptr;

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

        int seqNum = 0;

        /***********************************************************/
      
        fptr = fopen("sample_video.mp4", "rb");
        
        if (fptr == NULL){
            printf("Error opening file.\n");
            close(sock);
            return -1;
        }

        /***********************************************************/

        while (1) 
        {

            size_t num_read = fread(clientMessage, 1, MESSAGE_LENGTH, fptr);

            if ( num_read == 0 ){
                break;
            }

            //Creating packet using binn library so that it can be sent over the socket.
            binn *packet;
            packet = binn_object();

            // printf("Enter Message: ");
            // fgets(clientMessage, MESSAGE_LENGTH, stdin);
            binn_object_set_str(packet, "pay_load", clientMessage);
            binn_object_set_int32(packet, "seq_number", seqNum);

            int msg = sendto(sock, binn_ptr(packet), binn_size(packet), 0, (struct sockaddr *)&server, sizeof(server));

            if (msg >= 0)
            {
                printf("Sent message successfully.\n");

                int reply = recvfrom(sock, ack, sizeof(ack), 0, (struct sockaddr *)&server, &serverLength);
            
                if (binn_object_int32(ack, "seq_number") == seqNum) {
                    printf("Next sequence number required: %d\n", binn_object_int32(ack, "seq_number"));
                    fseek(fptr, -MESSAGE_LENGTH, SEEK_CUR);
                    continue;
                }


                if (reply >= 0)
                {
                    printf("Next sequence number required: %d\n", binn_object_int32(ack, "seq_number"));
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
            binn_free(packet);
            seqNum++;
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