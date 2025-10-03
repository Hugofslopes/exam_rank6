<p align="center">
    <img src="https://www.42porto.com/wp-content/uploads/2024/08/42-Porto-Horizontal.png" alt="Pipex Image" />
</p>
<h1 align="center">Exam rank 6</h1>
<p align="center">
    <img src="https://github.com/user-attachments/assets/64632116-be44-4c22-b0b0-0f4fd027f218" alt="Image"/>
</p>

<p>
This is the code I built for the Rank 6 exam. It creates a program that replicates the behavior of a <b>mini webserver</b>. I open sockets for each new client and then use <b>select</b> to manage all the fds. Each connected client receives a message when a new user joins or when another leaves. In addition, every message sent to the server is redistributed to all connected clients.
</p>
<p>
Using an infinite loop that contains an inner loop, the program iterates through each connected client and checks whether it's a new connection, has sent a message, or has disconnected. We identify a new client when the file descriptor (fd) is equal to the listening socket — the one created earlier. 
If the fd is not ready for reading, the loop simply continues. 
If the return value of recv is 0, it means the client has closed the connection gracefully. 
If it's -1, an error occurred (e.g., connection reset, interrupted system call). In both cases, the function to remove the client is called.
</p>
