# webserv
working on GET i need to read about URI again

i need to find a solution for that parsing and processing call inside the loop

i have to work on request state

i have to add exceptions for server error, and search if it is a 500 error code, or is it an exit and terminate server

cgi shit

===================================================================


// Step 1: Wait for ANY fd to be ready ready_fds = select/poll/epoll(all_fds);

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


==================================
multiplexing done (t9riban)

remove errno checks

client getters utils to be set after reading how request is processed

need to finish the client destroyer and delete in coreserver

update clients map

figure out efficient way to navigate into request sockets in buffer for reading

update the way to process the request and remove place holder

integrate into main and add automation tests cz they take time