//
// Created by marco on 11.10.22.
//
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <gtk/gtk.h>

#define READ 0
#define WRITE 1

void clientP2P(char *message,int port);
void serverP2P(int port);
int simpleClient(int port);
int simpleServer(int port);
void parent(int pfd_child1[2], int pfd_child2[2]);
void child1(int pfd[2], int port);
void child2(int pfd[2], int port);
void childReaper(pid_t childPID, int child_status);

// GUI
void do_display(GtkWidget *calculate, gpointer data);
void add_to_messageHistory(char *messageHistory, char *sender_message, char *receiver_message);

static GtkWidget *inputMessage;
static GtkWidget *result;
char messageHistory[3000]; // todo: dynamic memory

int main(int argc, char *argv[])
{
    int child1_status;
    int child2_status;

    char *message = malloc(255);

    //printf("Argv: %s\t", argv[1]);
    printf("Port input: %d\t", atoi(argv[1]));
    printf("Port output: %d\n", atoi(argv[2]));

    // pid_t for the two forks
    pid_t p1;
    pid_t p2;

    int pfd_child1[2]; // Used to store two ends of the pipe of child1
    int pfd_child2[2]; // Used to store two ends of the pipe of child2

    if (pipe(pfd_child1) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if (pipe(pfd_child2) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    if (p1 < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }
    if (p2 < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // parent process

    if ((p1 = fork()) > 0)
    {
        printf("Parent: My PID: %i\n", p1);
        // parent
        if ((p2 = fork() > 0))
        {
            // parent
            // acts as server
            printf("Parent: My PID: %i\n", p2);

            // --------------------------------------------------------------------------------------------
            // GUI
            GtkWidget *window, *grid, *display_message_history;
            gtk_init(&argc, &argv);

            window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

            grid = gtk_grid_new();
            gtk_container_add(GTK_CONTAINER(window), grid);

            inputMessage = gtk_entry_new();
            gtk_grid_attach(GTK_GRID(grid), inputMessage, 0, 10, 1, 1);

            display_message_history = gtk_button_new_with_label("send");
            g_signal_connect(display_message_history, "clicked", G_CALLBACK(do_display), NULL);
            gtk_grid_attach(GTK_GRID(grid), display_message_history, 2, 10, 1, 1);

            result = gtk_label_new("Messages:");
            gtk_grid_attach(GTK_GRID(grid), result, 0, 0, 3, 10);

            gtk_widget_show_all(window);
            gtk_main();

            // --------------------------------------------------------------------------------------------
            parent(pfd_child1, pfd_child2);
        }
        else
        {
            // child2

            //sleep(3);
            child2(pfd_child2, atoi(argv[2]));

            printf("Child finished\n");

            printf("I'm child2, my PID: %i", p2);


        }

    }
    else
    {
        // child1
        child1(pfd_child1, atoi(argv[1]));

        printf("I'm child1, my PID: %i", p1);




    }

    printf("Parent: I will reap the children in 300 seconds.\n");
    sleep(300);
    // reaps children
    childReaper(p1, child1_status);
    childReaper(p2, child2_status);



    /**
    if (strcmp(argv[1], "-client") == 0)
    {
        printf("\nClient started.....\n");
        clientP2P();
        printf("That's the stored message: %s", message);
    } else if (strcmp(argv[1], "-server") == 0)
    {
        printf("\nServer started.....\n");
        serverP2P();
    }
     **/
}

void childReaper(pid_t childPID, int child_status)
{
    printf("Kills child\n");
    kill(childPID, SIGINT);

    pid_t wpid = wait(&child_status);
    if (WIFEXITED(child_status))
    {
        printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));
    } else
    {
        printf("Child %d terminated abnormally\n", wpid);
    }
}

/**
 * Parent
 * Writes into pfd_child1
 * Reads from pfd_child2
 * @param pfd_child1
 * @param pdf_child2
 */
void parent(int pfd_child1[2], int pfd_child2[2])
{
    close(pfd_child1[WRITE]);
    close(pfd_child2[WRITE]);

    //char *bufferChild2 = (char*)malloc(256);
    char bufferChild2[256];

    //char *bufferChild1 = (char*)malloc(256); // buffer size: 256 bytes
    char bufferChild1[256];

    int counter = 0;
    // loop to read pipes
    while (1)
    {
        if (counter > 20)
        {
            break;
        }
        counter++;
        // reads from pipe
        read(pfd_child1[READ], bufferChild1, 256); // read 256 characters
        //printf("Received message from Child1: %s\n", bufferChild1);

        sleep(1);

        read(pfd_child2[READ], bufferChild2, 256);
        //printf("Received message from Child2: %s\n", bufferChild2);

        sleep(1);

        // the messages from the pipe are written into the array messageHistory
        add_to_messageHistory(messageHistory, bufferChild1, bufferChild2);

        // todo: if child sends signal x to parent => stops, breaks out of loop
    }

    // closes fd after it's not used anymore
    close(pfd_child1[READ]);
    close(pfd_child2[READ]);
}

/**
 * Sender
 * Write into pipe
 * @param pfd_child1
 * @param port
 */
void child1(int pfd_child1[2], int port)
{
    close(pfd_child1[READ]);
    printf("Child1: Server/Sender Port is %i\n", port);

    // Server
    // generates Socket, connected with client
    int socket = simpleServer(port);
    char buffer[1024];

    /*---- Send message to the socket of the incoming connection ----*/
    int count = 0;
    while (1) {
    {
        count++;
        if (count > 3)
        {
            printf("Finished with writing");
            break;
        }
    }
        printf("Enter message: ");
        fgets(buffer, 1024, stdin);
        printf("Got input: %s\n", buffer);
        write(pfd_child1[WRITE], buffer, sizeof(buffer));

        strcpy(buffer,"Hello World\n");
        send(socket,buffer,13,0);
    }

    close(pfd_child1[WRITE]);
}

/**
 * Receiver
 * Writes into pipe
 * @param pfd_child2
 * @param port
 */
void child2(int pfd_child2[2], int port)
{
    char *message[255];
    printf("Child2: Client/Receiver Port is %i\n", port);
    close(pfd_child2[READ]);

    simpleClient(port);

    write(pfd_child2[WRITE], message, sizeof(message));
    close(pfd_child2[WRITE]);
}

// both wait a random time with switching modus, random decided if server or client

// changes between server and client

/**
 * (Sender, Child1) Server sends messages to the client. Gets input through a pipe from the parent.
 * @param port
 */
int simpleServer(int port)
{
    int welcomeSocket, newSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(port);
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*---- Bind the address struct to the socket ----*/
    bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*---- Listen on the socket, with 5 max connection requests queued ----*/
    if(listen(welcomeSocket,5)==0)
        printf("Listening\n");
    else
        printf("Error\n");

    /*---- Accept call creates a new socket for the incoming connection ----*/
    addr_size = sizeof serverStorage;
    newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

    /*---- Send message to the socket of the incoming connection ----*/
    strcpy(buffer,"Hello World\n");
    send(newSocket,buffer,13,0);
    printf("Server established connection to the client and sent a message to client.\n");

    return newSocket;
}

/**
 * (Receiver, Child2) Client receives messages from the server. Sends received message through a pipe to the parent.
 * @param port
 */
int simpleClient(int port)
{
    int clientSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(port);
    /* Set IP address to localhost */
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*---- Connect the socket to the server using the address struct ----*/
    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

    /*---- Read the message from the server into the buffer ----*/
    recv(clientSocket, buffer, 1024, 0);
    printf("Client: received the message from the server.\n");

    /*---- Print the received message ----*/
    printf("Data received: %s",buffer);

    // How to send and recv
    //while (1)
    //{
        // recv() from server; => checks if it should stop the connection
        // send() from client;

    //}

    return clientSocket;
}

void serverP2P(int port)
{
    // create server socket similar to what was done in
    // client program
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    // string store data to send to client
    char serMsg[255] = "Message from the server to the "
                       "client \'Hello Client\' ";

    // define server address
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to the specified IP and port
    int bindValue = bind(servSockD, (struct sockaddr*)&servAddr,
         sizeof(servAddr));

    if (bindValue == -1)
    {
        printf("Binding port failed.\n");
    } else
    {
        // listen for connections
        listen(servSockD, 1);

        // integer to hold client socket.
        int clientSocket = accept(servSockD, NULL, NULL);

        // send's messages to client socket
        send(clientSocket, serMsg, sizeof(serMsg), 0);
        printf("Server: Sent message to client\n");
    }

}

void clientP2P(char *message, int port)
{
    int sockD = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port
            = htons(port); // use some unused port number
    servAddr.sin_addr.s_addr = INADDR_ANY;

    int connectStatus
            = connect(sockD, (struct sockaddr*)&servAddr,
                      sizeof(servAddr));

    if (connectStatus == -1) {
        //printf("Error...\n");
        //connect(sockD, (struct sockaddr*)&servAddr,sizeof(servAddr));
        int counter = 0;
        while (connectStatus == -1)
        {
            if (counter > 5)
            {
                printf("Connection wasn't possible\n");
                break;
            }
            counter++;
            printf("Couldn't connect, trying again, try number: %i \n", counter);
            connectStatus = connect(sockD, (struct sockaddr*)&servAddr,sizeof(servAddr));
        }

    }

    else {
        char strData[255];

        recv(sockD, strData, sizeof(strData), 0);

        printf("Message: %s\n", strData);
        // Allocates needed memory for storing the message in the heap
        //message = (char*)malloc(sizeof(strData) * sizeof(char));
        strncpy(message, strData, 255);
    }
}


// --------------------------------------------------------------------------------------------
// GUI Functions

void do_display(GtkWidget *calculate, gpointer data) {
    char *message = (char *)gtk_entry_get_text(GTK_ENTRY(inputMessage));


    // add input message to string
    // print new string into it
    printf("Message history: %s\n", messageHistory);
    printf("Message: %s\n", message);

    // todo: Eventuell nicht in messageHistory hineinschreiben sondern nur Nachrichten + Sender + Datum/Uhrzeit
    strcat(messageHistory, "\n-> new message\t:\n");

    strcat(messageHistory, message);

    printf("Message history: %s\n", messageHistory);
    printf("Message: %s\n", message);

    char buffer[strlen(messageHistory)];
    snprintf(buffer, sizeof(buffer), "Message: %s\n", messageHistory);

    gtk_label_set_text(GTK_LABEL(result), buffer);

    // clears input field
    gtk_entry_set_text(GTK_ENTRY(inputMessage), "");
}

/**
 *
 * @param size
 * @return 1 or 0
 */
int check_for_change(int size)
{

}

// todo: doesn't check if messageHistory Array is big enough
/**
 * Adds new input from sender and receiver into file messageHistory
 * @param p_sender
 * @param p_receiver
 * @return
 */
void add_to_messageHistory(char *messageHistory, char *sender_message, char *receiver_message)
{
    if (sender_message[0] != '\n')
    {
        // add into messageHistory
        strcat(messageHistory, sender_message);
    }
    if (receiver_message[0] != '\n')
    {
        // add into messageHistory
        strcat(messageHistory, receiver_message);
    }
}

void increaseSize()
{

}

// --------------------------------------------------------------------------------------------