# webserv
working on GET i need to read about URI again

i need to find a solution for that parsing and processing call inside the loop

i have to work on request state

i have to add exceptions for server error, and search if it is a 500 error code, or is it an exit and terminate server
 removed the substr in parse_request coz redundant

cgi shit

===============================================================
## Part 1: Handling (Preparation & Routing)
This part prepares the data inside your existing server architecture before running the script.

* Parse Query and Path: Split the URI. Extract everything after ? into QUERY_STRING. Identify the script file path for SCRIPT_NAME.
* Build Environment Array: Create a char* envp[] array populated with key-value strings (KEY=value) representing HTTP headers:
* REQUEST_METHOD (GET / POST)
   * CONTENT_LENGTH and CONTENT_TYPE (if a body exists)
   * PATH_INFO (the requested resource path)
* Create Inter-Process Pipes: Initialize two Unix pipes to send data to and receive data from the script:
* pipe(cgi_to_script) (for sending the POST body)
   * pipe(script_to_cgi) (for capturing the script's output)

------------------------------
## Part 2: Execution (Process & Non-Blocking I/O)
This part handles running the script safely without freezing your main server loop. [1] 

* Fork the Process: Call fork() to split your server into a parent and child process. [2, 3] 
* In the Child Process:
* Use dup2 to redirect standard input (stdin) to cgi_to_script[0].
   * Use dup2 to redirect standard output (stdout) to script_to_cgi[1].
   * Close all original pipe file descriptors.
   * Call execve() to run the script interpreter (like Python or PHP). [4, 5, 6, 7] 
* In the Parent Process:
* Close the child’s ends of the pipes immediately.
   * Write Body (POST only): Feed the HTTP request body into cgi_to_script[1] and close it so the script receives an EOF (End of File).
   * Register to Multiplexer: Add script_to_cgi[0] to your select/poll/epoll loop. Read the script output asynchronously as it becomes available.
   * Monitor and Reclaim: Track the child PID. If it times out, send kill(pid, SIGKILL). Otherwise, clean up the process using waitpid(pid, &status, WNOHANG). [8, 9, 10] 





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
need to add sig_handlers
timeout clients (shld do it before finishing CGI to not get confused)
keep the upload counter up to date whenever


