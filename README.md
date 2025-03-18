# C Chat Application

This project is a chat application developed in C that implements both a client and a server using TCP sockets. It supports messaging between users, conversation history, unseen messages, replies, and displaying online users. The application leverages the `select()` system call for multiplexing and POSIX threads (`pthread`) for handling multiple clients concurrently. It enables real-time communication between two users, allowing them to send and receive messages live simultaneously.

---

## Features

- **Client-Server Architecture:**  
  The application consists of a server that manages multiple client connections and a client that enables users to interact through a command-line interface.

- **Real-Time Messaging:**  
  Users can exchange messages in real time. The application supports live, bidirectional communication between two users, ensuring that messages are sent and received simultaneously.

- **Messaging:**  
  Users can send messages to one another. The server forwards messages to online users and stores messages for offline recipients.

- **Conversation History:**  
  Clients can request the conversation history with another user. The history is stored in a file named based on the two users involved.

- **Unseen Messages:**  
  If a recipient is offline, their messages are saved in a separate file. When the user reconnects, the server sends any unseen messages.

- **Reply Functionality:**  
  Users can reply to specific messages by referencing the message number.

- **Online Users List:**  
  The client can request a list of currently online users from the server.

- **Multi-threading and Multiplexing:**  
  The server uses POSIX threads to handle multiple clients concurrently and employs `select()` to monitor socket activity and standard input.

---

