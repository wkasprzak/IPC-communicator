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

#define MAX_MESSAGE_NAME_LENGTH 100
#define MAX_MESSAGE_LENGTH 200
#define MAX_LENGTH 20

#define MAX_NUM_OF_USERS 10
#define MAX_NUM_OF_SUBSCRIPTIONS 20
#define MAX_NUM_OF_MESSAGES_FOR_USER 10

struct Message {
    long msgType;
    int id;
    int type;
    int topicID;
    int topicType;
    char topicName[MAX_MESSAGE_NAME_LENGTH];
    char name[MAX_LENGTH];
    char text[MAX_MESSAGE_LENGTH];
} msgToClient,
msgFromClient;

struct News {
    char text[MAX_MESSAGE_LENGTH];
    int priority;
    int topicID;
};

int handleLogin(char ***users, struct Message *msgFromClient) {

    int loginQueue = msgget(0x125, 0666 | IPC_CREAT);
    int flag = 0;

    msgToClient.msgType = msgFromClient->id;

    for(int i = 0; i < MAX_NUM_OF_USERS; i++) {
        if(strcmp(msgFromClient->name, users[i][0]) == 0) {
            strncpy(msgToClient.text, "The username is already used.", MAX_MESSAGE_LENGTH);
            msgsnd(loginQueue, &msgToClient, sizeof(struct Message), 0);
            flag = 1;
            return 1;
        } else if(strcmp(users[msgFromClient->id-1][0], "")) {
            strncpy(msgToClient.text, "The ID is already taken.", MAX_MESSAGE_LENGTH);
            msgsnd(loginQueue, &msgToClient, sizeof(struct Message), 0);
            flag = 1;
            return 1;
        }
    }

    if(!flag)
        if (strlen(msgFromClient->name) <= 0) {
            strncpy(msgToClient.text, "The username is too short.", MAX_MESSAGE_LENGTH);
            msgsnd(loginQueue, &msgToClient, sizeof(struct Message), 0);
            return 1;
        } else {
            strncpy(msgToClient.text, "Login successful", MAX_MESSAGE_LENGTH);
            msgsnd(loginQueue, &msgToClient, sizeof(struct Message), 0);
            return 0;
        }
    return 1;
}

int handleSubscription(char ***users, int clientsQueuesIDs[], char *topicsNames[], int subscriptions[][MAX_NUM_OF_USERS][MAX_NUM_OF_SUBSCRIPTIONS], struct Message *msgFromClient) {
    msgToClient.msgType = msgFromClient->id;
    for (int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++)
        if(strcmp(topicsNames[i], msgFromClient->topicName) == 0) {
            if(subscriptions[msgFromClient->topicType-1][msgFromClient->id-1][i] == 0) {
                strcpy(msgToClient.text, "Successfully subscribed.");
                msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
            } else {
                strcpy(msgToClient.text, "You already subscribe this topic");
                msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
                return 0;
            }
            return i;
        }
    strcpy(msgToClient.text, "This topic doesn't exist.");
    msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
    return 0;
}

int newTopic(int clientsQueuesIDs[], char *topicsNames[], struct Message *msgFromClient) {

    msgToClient.msgType = msgFromClient->id;
    for(int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++) {
        if(strcmp(topicsNames[i], msgFromClient->topicName) == 0) {
            strcpy(msgToClient.text, "The topic name is already in use.");
            msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message), 0);
            return 0;
        }
    }
    if(strcmp(topicsNames[msgFromClient->topicID - 1], "") != 0) {
        strcpy(msgToClient.text, "The ID is already taken.");
        msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message), 0);
        return 0;
    } else {
        strcpy(msgToClient.text, "Created new topic successfully");
        msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message), 0);
        return 1;
    }
}

int newMessage(char ***users, int clientsQueuesIDs[], char *topicsNames[], int subscriptions[][MAX_NUM_OF_USERS][MAX_NUM_OF_SUBSCRIPTIONS], struct News newsArray[][MAX_NUM_OF_USERS][MAX_NUM_OF_MESSAGES_FOR_USER], struct Message *msgFromClient) {

    msgToClient.msgType = msgFromClient->id;

    int topic = -1;
    for(int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++)
         if(strcmp(topicsNames[i], msgFromClient->topicName) == 0)
             topic = i;

    if(topic == -1) {
         strcpy(msgToClient.text, "The topic doesn't exist.");
         msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
         return 0;
    }

    struct News newsOnTopic;

    if(subscriptions[0][msgFromClient->id - 1][topic] != 0) {

         strncpy(msgToClient.text, "Message sent.", MAX_MESSAGE_LENGTH);
         msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);

         strncpy(newsOnTopic.text, msgFromClient->text, sizeof(newsOnTopic.text));
         printf("%s\n", newsOnTopic.text);
         newsOnTopic.priority = msgFromClient->topicID;
         newsOnTopic.topicID = topic;

         for(int i = 0; i < MAX_NUM_OF_USERS; i++) {
             int flag = 1;
             if(strcmp(users[i][msgFromClient->id - 1], "1") != 0)
                if(subscriptions[0][i][topic] < 12 && subscriptions[0][i][topic] > 0) {

                    int msgID = 0;
                    while (strcmp(newsArray[0][i][msgID].text, "")) {
                        msgID++;
                        if(msgID >= MAX_NUM_OF_MESSAGES_FOR_USER) {
                            flag = 0;
                            break;
                        }
                    }

                    if(flag) {
                        newsArray[0][i][msgID] = newsOnTopic;

                        if(subscriptions[0][i][topic] != 11)
                            subscriptions[0][i][topic]--;
                        if(subscriptions[0][i][topic] == 0)
                            subscriptions[1][i][topic] = 0;
                    }
                } else if(subscriptions[1][i][topic] > 0) {

                    int msgID = 0;
                    while (strcmp(newsArray[1][i][msgID].text, "")) {
                        msgID++;
                        if(msgID >= MAX_NUM_OF_MESSAGES_FOR_USER) {
                            flag = 0;
                            break;
                        }
                    }

                    if(flag) {
                        newsArray[1][i][msgID] = newsOnTopic;

                        if(subscriptions[1][i][topic] != 11)
                            subscriptions[1][i][topic]--;
                        if(subscriptions[1][i][topic] == 0)
                            subscriptions[0][i][topic] = 0;
                    }
                }
            }
     } else {
         strcpy(msgToClient.text, "You don't subscribe this topic.");
         msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
         return 0;
     }

     return 0;
}

void blockUser(char ***users, int clientsQueuesIDs[], struct Message *msgFromClient) {
    msgToClient.msgType = msgFromClient->id;
    char name[MAX_LENGTH];
    int a = 0;
    for(int i = 0; i < MAX_NUM_OF_USERS; i++) {
        if(!strcmp(users[i][0], msgFromClient->name)) {
            if(strcmp(users[msgFromClient->id - 1][i], "") == 0) {
                users[msgFromClient->id - 1][i] = "1";
                strcpy(msgToClient.text, "User blocked.");
                msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
                a = 1;
                break;
            } else {
                strcpy(msgToClient.text, "This user is already blocked.");
                msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
                a = 1;
                break;
            }
        }
    }

    if(!a) {
        strcpy(msgToClient.text, "This user doesn't exist");
        msgsnd(clientsQueuesIDs[msgFromClient->id-1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
    }
}

void fixUp(char ***users, int clientsQueuesIDs[], int subscriptions[][MAX_NUM_OF_USERS][MAX_NUM_OF_SUBSCRIPTIONS], struct News newsArray[][MAX_NUM_OF_USERS][MAX_NUM_OF_MESSAGES_FOR_USER], struct Message *msgFromClient) {

    for(int i = 0; i < MAX_NUM_OF_USERS; i++) {
        users[msgFromClient->id - 1][i] = "";
        users[i][msgFromClient->id - 1] = "";
        subscriptions[0][msgFromClient->id - 1][i] = 0;
        subscriptions[1][msgFromClient->id - 1][i] = 0;
    }

    for(int k = 0; k < 2; k++){
        for(int i = 0; i < MAX_NUM_OF_MESSAGES_FOR_USER; i++) {
            newsArray[k][msgFromClient->id-1][i].priority = 0;
            strcpy(newsArray[k][msgFromClient->id-1][i].text, "");
            newsArray[k][msgFromClient->id-1][i].topicID = 0;
        }
    }
    clientsQueuesIDs[msgFromClient->id - 1] = 0;
}

void sendMessage(char ***users, int clientsQueuesIDs[],char *topicsNames[], int subscriptions[][MAX_NUM_OF_USERS][MAX_NUM_OF_SUBSCRIPTIONS], struct News newsArray[][MAX_NUM_OF_USERS][MAX_NUM_OF_MESSAGES_FOR_USER], struct Message *msgFromClient) {

    msgToClient.msgType = msgFromClient->id;
    int numOfMsg, maxPriority, flag, msgSend = 0;

    int topic = -1;
    for(int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++)
         if(strcmp(topicsNames[i], msgFromClient->topicName) == 0)
             topic = i;

    if(topic != -1) {
        do {
            numOfMsg = 0, maxPriority = 0;
            for(int i = 0; i < MAX_NUM_OF_MESSAGES_FOR_USER; i++) {
                if(newsArray[0][msgFromClient->id-1][i].topicID == topic) {
                    numOfMsg++;
                    if(newsArray[0][msgFromClient->id-1][i].priority > maxPriority) {
                        strcpy(msgToClient.text, newsArray[0][msgFromClient->id - 1][i].text);
                        maxPriority = newsArray[0][msgFromClient->id-1][i].priority;
                        flag = i;
                    }
                }
            }
            if(numOfMsg > 0) {
                msgSend = 1;
                msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
                newsArray[0][msgFromClient->id-1][flag].priority = 0;
                strcpy(newsArray[0][msgFromClient->id-1][flag].text, "");
                newsArray[0][msgFromClient->id-1][flag].topicID = 0;
            }
        } while(numOfMsg);
    }

    if(subscriptions[1][msgFromClient->id -1][topic] == 0 && !msgSend){
        strcpy(msgToClient.text, "You don't subscribe this topic");
        msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
    }
    else
    {
        strcpy(msgToClient.text, "You don't have any more messages");
        msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
    }
}

void sendAsMessage(char ***users, int clientsQueuesIDs[], int newTopicInf[], char *topicsNames[], int subscriptions[][MAX_NUM_OF_USERS][MAX_NUM_OF_SUBSCRIPTIONS], struct News newsArray[][MAX_NUM_OF_USERS][MAX_NUM_OF_MESSAGES_FOR_USER], struct Message *msgFromClient) {
    msgToClient.msgType = msgFromClient->id;
    int numOfMsg, maxPriority, flag;

    if(newTopicInf[msgFromClient->id -1]){
        strcpy(msgToClient.text, "A new topic has been created");
        msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
        newTopicInf[msgFromClient->id -1] = 0;
    }

    do {
        numOfMsg = 0, maxPriority = 0;
        for(int i = 0; i < MAX_NUM_OF_MESSAGES_FOR_USER; i++) {
            if(strcmp(newsArray[1][msgFromClient->id-1][i].text, "")) {
                numOfMsg++;
                if(newsArray[1][msgFromClient->id-1][i].priority > maxPriority) {
                    strcpy(msgToClient.text, newsArray[1][msgFromClient->id - 1][i].text);
                    maxPriority = newsArray[1][msgFromClient->id-1][i].priority;
                    flag = i;
                }
            }
        }
        if(numOfMsg > 0) {
            msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
            newsArray[1][msgFromClient->id-1][flag].priority = 0;
            strcpy(newsArray[1][msgFromClient->id-1][flag].text, "");
            newsArray[1][msgFromClient->id-1][flag].topicID = 0;
        }
    } while(numOfMsg);

    strcpy(msgToClient.text, "You don't have any more messages");
    msgsnd(clientsQueuesIDs[msgFromClient->id - 1], &msgToClient, sizeof(struct Message) - sizeof(long), 0);
}


int main(int argc, char *argv[]) {

    int toServer = msgget(0x123, IPC_CREAT | 0666);

    int userCount = 0;
    int numOfRows = MAX_NUM_OF_USERS;
    int numOfCols = MAX_NUM_OF_USERS + 1;
    int clientsQueuesIDs[MAX_NUM_OF_USERS] = {0};
    int newTopicInf[MAX_NUM_OF_USERS] = {0};

    while ((msgrcv(toServer, &msgToClient, sizeof(struct Message), 0, IPC_NOWAIT)) != -1) {}

    char ***users = (char ***)malloc(numOfRows * sizeof(char **));
    for (int i = 0; i < numOfRows; i++) {
        users[i] = (char **)malloc(numOfCols * sizeof(char *));
        for (int j = 0; j < numOfCols; j++)
            users[i][j] = (char *)malloc(MAX_LENGTH * sizeof(char));
     }

    int topicsCount = 0;
    char *topicsNames[MAX_NUM_OF_SUBSCRIPTIONS];

    for (int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++)
        topicsNames[i] = malloc(MAX_MESSAGE_NAME_LENGTH * sizeof(char));

    int subscriptions[2][numOfRows][MAX_NUM_OF_SUBSCRIPTIONS];
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < numOfRows; j++)
            for(int k = 0; k < MAX_NUM_OF_SUBSCRIPTIONS; k++)
                subscriptions[i][j][k] = 0;

    struct News newsArray[2][MAX_NUM_OF_USERS][MAX_NUM_OF_MESSAGES_FOR_USER];
    for(int k = 0; k < 2; k++)
        for(int i = 0; i < MAX_NUM_OF_USERS; i++)
            for(int j = 0; j < MAX_NUM_OF_MESSAGES_FOR_USER; j++) {
                strncpy(newsArray[k][i][j].text, "", sizeof(newsArray[k][i][j].text));
                newsArray[k][i][j].priority = 0;
                newsArray[k][i][i].topicID = 0;
        }

    while (1) {
        for(int k = 0; k < 2; k++){
            for(int i = 0; i < MAX_NUM_OF_USERS; i++){
                for(int j = 0; j < MAX_NUM_OF_MESSAGES_FOR_USER; j++)
                    printf("%s %d;", newsArray[k][i][j].text, newsArray[k][i][j].priority);
                printf("\n");
            }
            printf("\n");
        }
        printf("\n");

        printf("Received a message with type %ld\n", msgFromClient.msgType);
        printf("Users and subscriptions:\n");
        for (int i = 0; i < numOfRows; i++) {
            for (int j = 0; j < numOfCols; j++)
                printf("%s ", users[i][j]);
            printf("\n");
        }

        printf("\nCurrent topics:\n");
        for (int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++)
            printf("%d. %s\n", i+1, topicsNames[i]);

        msgFromClient.msgType = 0;
        msgrcv(toServer, &msgFromClient, sizeof(struct Message), 0, 0);

        switch (msgFromClient.msgType) {
            case 1:
                // Login
                printf("Handling logging...\n");
                if(!handleLogin(users, &msgFromClient)) {
                    userCount++;
                    key_t key = (key_t)(msgFromClient.id);
                    clientsQueuesIDs[msgFromClient.id - 1] = msgget(key, 0666 | IPC_CREAT);
                    char idStr[MAX_LENGTH];
                    strncpy(users[msgFromClient.id-1][0], msgFromClient.name, MAX_LENGTH);
                }
                break;
            case 2:
                // subscription
                printf("Handling subscription...\n");
                int subscriptionId = handleSubscription(users, clientsQueuesIDs, topicsNames, subscriptions, &msgFromClient);
                if(subscriptionId > 0) {
                    if(msgFromClient.topicType-1) {
                        subscriptions[1][msgFromClient.id-1][subscriptionId] = msgFromClient.type;
                        subscriptions[0][msgFromClient.id-1][subscriptionId] = 12;
                    } else {
                        subscriptions[0][msgFromClient.id-1][subscriptionId] = msgFromClient.type;
                        subscriptions[1][msgFromClient.id-1][subscriptionId] = 12;
                    }
                }
                break;
            case 3:
                // new topic
                printf("Creating new topic...\n");
                if(newTopic(clientsQueuesIDs, topicsNames, &msgFromClient)) {
                    topicsCount++;
                    strncpy(topicsNames[msgFromClient.topicID - 1], msgFromClient.topicName, MAX_MESSAGE_NAME_LENGTH);
                    msgToClient.msgType = msgFromClient.id;
                    strcpy(msgToClient.text, "A new topic has been created.");

                    for(int i = 0; i < MAX_NUM_OF_USERS; i++)
                        if(strcmp(users[i][0], ""))
                            newTopicInf[i] = 1;
                }
                break;
            case 4:
                // new message
                printf("Recieving a message...\n");
                newMessage(users, clientsQueuesIDs, topicsNames, subscriptions, newsArray, &msgFromClient);
                break;
            case 5: // synchronous sending
                printf("Sending messages...\n");
                sendMessage(users, clientsQueuesIDs, topicsNames, subscriptions, newsArray, &msgFromClient);
                break;
            case 6: // user list
                printf("Sending users list...\n");
                msgToClient.topicID = userCount;
                msgToClient.msgType = msgFromClient.id;
                msgsnd(clientsQueuesIDs[msgFromClient.id - 1], &msgToClient, sizeof(struct Message), 0);
                for(int i = 0; i < MAX_NUM_OF_USERS; i++)
                    if(strcmp(users[i][0], "")) {
                        strcpy(msgToClient.text, users[i][0]);
                        msgToClient.topicID = i + 1;
                        msgsnd(clientsQueuesIDs[msgFromClient.id - 1], &msgToClient, sizeof(struct Message), 0);
                    }
                break;
            case 7: // block
                printf("Blocking user...\n");
                blockUser(users, clientsQueuesIDs, &msgFromClient);
                break;
            case 8: // user left
                printf("Fixing up...\n");
                fixUp(users, clientsQueuesIDs, subscriptions, newsArray, &msgFromClient);
                userCount--;
                break;
            case 10:
                // sending topics list
                printf("Sending topics list...\n");
                msgToClient.topicID = topicsCount;
                msgToClient.msgType = msgFromClient.id;
                msgsnd(clientsQueuesIDs[msgFromClient.id - 1], &msgToClient, sizeof(struct Message), 0);
                for(int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS; i++)
                    if(strcmp(topicsNames[i], "")) {
                        strcpy(msgToClient.text, topicsNames[i]);
                        msgsnd(clientsQueuesIDs[msgFromClient.id - 1], &msgToClient, sizeof(struct Message), 0);
                    }
                break;
            case 11: // asynchronous sending
                printf("Sending messages...\n");
                sendAsMessage(users, clientsQueuesIDs, newTopicInf, topicsNames, subscriptions, newsArray, &msgFromClient);
                break;
            default:
                printf("Unknown message type: %ld\n", msgFromClient.msgType);
                break;
        }
    }

    for (int i = 0; i < numOfRows; i++){
        for (int j = 0; j < numOfCols; j++)
            free(users[i][j]);
        free(users[i]);
    }
    free(users);

    for (int i = 0; i < MAX_NUM_OF_SUBSCRIPTIONS+1; i++)
        free(topicsNames[i]);

    msgctl(toServer, IPC_RMID, NULL);
}
