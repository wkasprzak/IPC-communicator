#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<ctype.h>
#include<fcntl.h>
#include<signal.h>
#include<string.h>

#define MAX_LENGTH 20
#define MAX_MESSAGE_NAME_LENGTH 100
#define MAX_MESSAGE_LENGTH 200

struct Message {
    long msgType;
    int id;
    int type;
    int topicID;
    int topicType;
    char topicName[MAX_MESSAGE_NAME_LENGTH];
    char name[MAX_LENGTH];
    char text[MAX_MESSAGE_LENGTH];
};

int main(int argc, char *argv[]) {

    int toServer = msgget(0x123, 0666 | IPC_CREAT);
    int loginQueue = msgget(0x125, 0666 | IPC_CREAT);
    char name[MAX_LENGTH] = "";
    int userID;
    key_t key;

    while(1) {

        struct Message  msgToServer, msgFromServer;

        msgToServer.msgType = 1;
        printf("Please enter the username (up to 100 characters): ");
        fgets(msgToServer.name, MAX_LENGTH, stdin);
        msgToServer.name[strcspn(msgToServer.name, "\n")] = '\0';
        fflush(stdin);

        printf("Please enter the ID number (a number from 1 to 10): ");
        scanf("%d", &msgToServer.id);
        while(getchar() != '\n');
        fflush(stdin);

        if(msgToServer.id < 11 && msgToServer.id > 0) {

            msgsnd(toServer, &msgToServer, sizeof(struct Message), 0);
            msgFromServer.msgType = msgToServer.id;
            msgrcv(loginQueue, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);

            if (strcmp(msgFromServer.text, "Login successful") == 0) {
                msgctl(loginQueue, IPC_RMID, NULL);
                strncpy(name, msgToServer.name, MAX_LENGTH);
                userID = msgToServer.id;
                key = (key_t)(msgToServer.id);
                break;
            } else
                printf("Server response: Incorrect data. Please, try again\n");
        } else
            printf("ID number must be between 1 and 10. Please, try again\n");
    }

    system("clear");

    int fromServer = msgget(key, 0666 | IPC_CREAT);
    int action;
    int close = 0;

        while(!close) {

            struct Message  msgToServer, msgFromServer;

            msgToServer.msgType = 11;
            printf("\nYour messages: \n");
            msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
            do {
                msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id , 0);
                printf("%s \n", msgFromServer.text);
            } while(strcmp(msgFromServer.text, "You don't have any more messages"));

            if(msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, IPC_NOWAIT) > 0)
                printf("%s \n", msgFromServer.text);

            printf("\n=== OPTIONS ===\n1) See the list of topics\n2) Subscribe to the topic\n3) Create a new topic\n4) Send a message\n5) Check messages\n6) See the list of users\n7) Block user\n8) Exit\n\nYour choice (write number between 1 and 8): ");
            scanf("%d", &action);
            while(getchar() != '\n');

            system("clear");

        switch(action) {
            case 1: // topics list
                msgToServer.msgType = 10;
                msgsnd(toServer, &msgToServer, sizeof(struct Message), 0);

                msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                int topics = msgFromServer.topicID;
                printf("\nCurrent topics:\n");
                while(topics != 0) {
                    msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                    printf("%s\n", msgFromServer.text);
                    topics--;
                }
                break;


            case 2: // subscribe topic
                msgToServer.msgType = 2;

                printf("Please enter name of the topic which you want to subscribe: ");
                fgets(msgToServer.topicName, MAX_LENGTH, stdin);
                msgToServer.topicName[strcspn(msgToServer.topicName, "\n")] = '\0';
                fflush(stdin);

                printf("If you want to subscribe to %s temporarily, write the number of messages you want to receive (a number from 1 to 10). If permanent, write 11: ", msgToServer.topicName);
                scanf("%d", &msgToServer.type);
                while(getchar() != '\n');

                printf("Please enter the type of the subscription (1 - if synchronous, 2 - if asynchronous): ");
                scanf("%d", &msgToServer.topicType);
                while(getchar() != '\n');

                if(msgToServer.type < 12 && msgToServer.type > 0) {
                    if(msgToServer.topicType < 3 && msgToServer.topicType > 0) {

                        msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
                        msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);

                        if (strcmp(msgFromServer.text, "Successfully subscribed.") == 0)
                            printf("You are now subscribing to the topic %s!\n", msgToServer.topicName);
                        else
                            printf("Server response: %s Please, try again\n", msgFromServer.text);
                    } else
                        printf("Type of topic must be between 1 and 2. Please, try again\n");
                }
                else
                    printf("Incorrect type of subscription. Please, try again\n");
                break;
            case 3: // adding topic
                msgToServer.msgType = 3;

                printf("Please enter the ID of the new topic (a number from 1 to 20): ");
                scanf("%d", &msgToServer.topicID);
                while(getchar() != '\n');

                printf("Please write name of new topic: ");
                fgets(msgToServer.topicName, MAX_MESSAGE_NAME_LENGTH, stdin);
                msgToServer.topicName[strcspn(msgToServer.topicName, "\n")] = '\0';
                fflush(stdin);

                if(msgToServer.topicID < 21 && msgToServer.topicID > 0) {
                    msgsnd(toServer, &msgToServer, sizeof(struct Message), 0);
                    msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);

                    if (strcmp(msgFromServer.text, "Created new topic successfully") == 0)
                        printf("%s\n", msgFromServer.text);
                    else
                        printf("Server response: %s Please, try again\n", msgFromServer.text);
                } else
                    printf("Topic ID must be between 1 and 20. Please, try again\n");
                break;
            case 4:
                msgToServer.msgType = 4;
                printf("Please enter the name of the topic for which you want to send a message: ");
                fgets(msgToServer.topicName, MAX_LENGTH, stdin);
                msgToServer.topicName[strcspn(msgToServer.topicName, "\n")] = '\0';
                fflush(stdin);

                printf("Please enter the priority of the message (1-10): ");
                scanf("%d", &msgToServer.topicID);
                while(getchar() != '\n');

                printf("Please enter the message:\n");
                fgets(msgToServer.text, MAX_MESSAGE_LENGTH, stdin);
                msgToServer.text[strcspn(msgToServer.text, "\n")] = '\0';

                if(msgToServer.topicID < 11 && msgToServer.topicID > 0 ) {
                    msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
                    msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                    if (strcmp(msgFromServer.text, "Message sent.") == 0)
                        printf("You sent a message successfully\n");
                    else
                        printf("Server response: %s Please, try again\n", msgFromServer.text);
                } else
                    printf("Priority of the message must be between 1 and 10.\n");
                break;
            case 5: // synchronous reception
                msgToServer.msgType = 5;
                printf("Please enter the name of the topic for which you would like to receive a message: ");
                fgets(msgToServer.topicName, MAX_MESSAGE_NAME_LENGTH, stdin);
                msgToServer.topicName[strcspn(msgToServer.topicName, "\n")] = '\0';

                printf("Would you like to wait if there is no new message? (0 - no, 1 -yes): ");
                scanf("%d", &msgToServer.topicID);
                while(getchar() != '\n');

                printf("\nYour messages: \n");

                if(msgToServer.topicID == 0){

                    msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
                    do {
                        msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                        printf("%s \n", msgFromServer.text);
                    } while(strcmp(msgFromServer.text, "You don't have any more messages") && strcmp(msgFromServer.text, "You don't subscribe this topic"));
                }
                else
                {
                    int flag = 0;
                    msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
                    do {
                        msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                        if(strcmp(msgFromServer.text, "You don't subscribe this topic") == 0){
                            printf("%s \n", msgFromServer.text);
                            break;
                        }
                        else if(strcmp(msgFromServer.text, "You don't have any more messages") == 0){
                            if(flag) break;
                            sleep(2);
                            msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
                        }
                        else{
                            printf("%s \n", msgFromServer.text);
                            flag = 1;
                        }
                    } while(1);
                }
                break;
            case 6: // user list
                msgToServer.msgType = 6;
                msgToServer.id = userID;
                msgsnd(toServer, &msgToServer, sizeof(struct Message), 0);
                msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                int users = msgFromServer.topicID;
                printf("\nUsers:\n\n");
                for (int i = 0; i < users; i++) {
                    msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                    printf("%d. %s\n", i + 1, msgFromServer.text);
                }
                break;
            case 7: // block a user
                msgToServer.msgType = 7;
                printf("To block a user enter the username: ");
                fgets(msgToServer.name, MAX_LENGTH, stdin);
                msgToServer.name[strcspn(msgToServer.name, "\n")] = '\0';
                fflush(stdin);
                if(strcmp(msgToServer.name, name) != 0) {
                    msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);

                    msgrcv(fromServer, &msgFromServer, sizeof(struct Message), msgToServer.id, 0);
                    if (strcmp(msgFromServer.text, "User blocked.") == 0)
                        printf("You blocked %s\n", msgToServer.name);
                    else
                        printf("Server response: %s Please, try again\n", msgFromServer.text);
                } else
                    printf("You cannot block yourself.\n");

                break;
            case 8:
                msgToServer.msgType = 8;
                msgsnd(toServer, &msgToServer, sizeof(struct Message) - sizeof(long), 0);
                printf("Thanks for using of our system. Your account will be deleted from server's database.\n");
                close = 1;
                break;
            default:
                printf("Unknown message type: %d\nTry again\n", action);
                break;
            }
        }

    msgctl(fromServer, IPC_RMID, NULL);
}
