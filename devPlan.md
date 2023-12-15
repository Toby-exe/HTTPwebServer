# How we handled development

## Tech Used For Collaboration And Development

We used `Github` to store our code and to collaborate on it. 

We used `Discord` to communicate with each other and to share our screens when we needed help or want to do some pair programming.

We used `Visual Studio Code` as our IDE via the Microsoft ssh extension to connect to our `Ubuntu` vm's.

## Devlopment Timeline

### Week 1 - Research

#### Goals

1. Learn more about how networking and servers work at a high level.
2. Experiement with browsers like `Google Chrome` to understand the relationship between a client and a server (inspect element network tab)
3. Research Internet Protocols, specifically HTTP.

#### Achieved 
During this week, using various resources like articles, GitHub repository's, YouTube videos, and Linux's manual pages, we learned
whatever we could about what it takes to write your own web server. This week's progress on the implementation end was very minimal but our research on concepts like ports, sockets, connections, etc gave us a much better idea of the scope of this project and helped us determine what features would be needed for basic version.

### Week 2 - Planning

#### Goals: 

1. Get an overview of the elements that make up a server and how to distribute the work load
2. Research and experiment with POSIX's <sys/socket.h> library.
3. Start implementing functionality need by any a server such as listening to a port on a socket, accepting a connection and handling a client request.
4. Research what additional features we wish to add 

#### Achieved: 
In the second week, we started by making a rudamentary client to see the opposite side of the server-client relationship. It connected to a given IP from a server listening on port 80. We could simply ping something like google.com to request it to send it's root page using HTTP GET. After getting hands-on with the client side, we worked on implementing a basic version a web server. 
It simply created a new socket that we bound to port 18000 that listened for connections. If any connections were accepted, we simulated processing a request by responding with whatever the client sent to the server.

### Week 3

#### Goals: 

1. Start actually servicing requests rather than simulating the process.
2. Create a more sophisticated parser using regular expressions to get the components of a request
3. Start expanding the capabilities of the shell including handling more types of HTTP requests.
4. Start working on a cache for frequently-requested pages.

#### Achieved: 
During the third week, 
We continued modifying the basic version we started in the previous week. Now, instead of just sending a request back to the client, we actually started figuring out what makes up a request and what we need to do with each component. As we got parsing and servicing working, we started creating pages and application that would be hosted on our server. Starting with a basic landing page, we found html, js, and css, to actually see how the server would work in a practical environment. This lead to a basic chat application which required us to implement POST for uploading messages after getting them.

We also realized that if there are enough clients requesting the same page from the server, it would get significantly slower. So we got the idea to implement an LRU cache as an extra feature used for optimization.
### Week 4

#### Goals: 

1. Add better error handling for the server
2. Create an O(1) method to retrieve MIME types
3. Make the server multithreaded
4. Find a way to retrieve performance statistics

#### Achieved: 
During the fourth week, we added better error handling for both the server and utility libraries. In our main, rather than calling our request handler function in the main loop right after accepting a connection, we created and detached a worker thread to complete that task instead. The improvement in performance wasn't something we saw immediately but we realized that without multithreading, the server would come to a halt after recieving a long request. We also began looking for better ways to test the server. We came up with a way to display things like response time. 

### Week 5

#### Goals: 

1. Implement a thread pool 
2. Persistant connections with keep-alive

#### Achieved: 
With our messaging app, we realized our old logic of making a new client for every request didn't really make sense. So we made use of the keep-alive component that Google Chrome uses to keep a client connected until a timeout. This also meant only one thread would be used per client accessing the messaging app.

We realized that the overhead of creating a new thread for every new request would eventually become a problem. Our thread pool solved this problem by pre-creating threads that will be used for all request handling until the server shuts down. 

## How We Divided the Work

| Name             | Tasks                                                                 |
|------------------|-----------------------------------------------------------------------|
| `Jayden Mingle`    | Custom Linked List library, Custom Hash Table library, Custom LRU Cache, Custom Queue library, file extension to MIME type conversion, Custom Dynamic Thread Pool library, Utilities Test Plan |
| `Tobias Wondwossen`| Server, Request Handler, Request Parser, Tests, Web App, Server Test Plan |
| `All`    | Documentation, Planning, Project Managment|