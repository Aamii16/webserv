# webserv
working on GET i need to read about URI again

i need to find a solution for that parsing and processing call inside the loop

i have to work on request state

i have to add exceptions for server error, and search if it is a 500 error code, or is it an exit and terminate server
 removed the substr in parse_request coz redundant

cgi shit

===============================================================
Here are the 4 simple steps to implement CGI:
## 1. Set Up Environment Variables
Create an array of strings (char* env[]) containing HTTP details for the script:

* REQUEST_METHOD (GET, POST)
* QUERY_STRING (everything after ? in URL)
* CONTENT_LENGTH & CONTENT_TYPE (for POST)
* SCRIPT_NAME (path to the script)

## 2. Create Pipes and Fork
Setup communication channels and spawn a child process:

* Create two pipes: pipe(pipe_in) and pipe(pipe_out).
* Call fork().
* In Child: Use dup2 to redirect stdin to pipe_in[0] and stdout to pipe_out[1]. Close unused pipe ends, then run the script using execve().
* In Parent: Close pipe_in[0] and pipe_out[1].

## 3. Write Body & Read Output (Non-Blocking)
Pass request data to the script and capture the result:

* Write the HTTP request body into pipe_in[1]. Close it immediately after so the script knows you are done sending data.
* Add pipe_out[0] to your select/poll/epoll loop.
* Read the script's output from pipe_out[0] only when data is ready. Track time to kill the process (SIGKILL) if it hangs.

## 4. Parse CGI Response
The script returns a mini-header (e.g., Status: 200 OK\r\nContent-Type: text/html\r\n\r\n<body>).

* Parse the script's Status line.
* Prepend a standard HTTP/1.1 <Status> line.
* Forward the combined headers and body back to the client socket.

Would you like a minimal C++ code template showing exactly how to execute the pipe, fork, and execve sequence?



===================================================================


multiplexing done (t9riban)

remove errno checks

client getters utils to be set after reading how request is processed

need to finish the client destroyer and delete in coreserver

update clients map

figure out efficient way to navigate into request sockets in buffer for reading

update the way to process the request and remove place holder

integrate into main and add automation tests cz they take time
GOTTA WORK ON CONNECTION LOOOP 
while (true) {
    // Step 1: Wait for ANY fd to be ready
    ready_fds = select/poll/epoll(all_fds);
    
    for (each ready_fd in ready_fds) {
        
        // Step 2: Is it a new connection?
        if (ready_fd == listen_socket)
            new_fd = accept();
            add new_fd to monitored fds;
        
        // Step 3: Is it an existing connection with data?
        else {
            buffer = read(ready_fd);
            
            // Step 4: Process request
            state = handler.process(buffer);
            
            // Step 5: Request complete?
            if (state == COMPLETE) {
                response = handler.getResponse();
                write(ready_fd, response);
                
                // Step 6: Keep connection open?
                if (handler.shouldClose())
                    close(ready_fd);
                    remove ready_fd from monitored fds;
                else
                    handler.reset();  // Clear for next request
            }
            else if (state == ERROR) {
                write(ready_fd, error_response);
                close(ready_fd);
            }
        }
    }
}


--------------------------------------------------------------------------------------------------------------
-ERRORS

GET got fixed in last commit

POST still have same error: it throws error 500 internal server error 
"printf 'POST / HTTP/1.1\r\nHost: localhost' | nc localhost 8080"

chunked requests still to be handled :
"printf 'POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\n\r\n' | nc localhost 8080"

program segfaults when a non implemeted method is used by client "PATCH"

also when the request is in lowercase


