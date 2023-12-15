# Interactive Test Plan for HTTP Server

## Getting Started

To ensure consistency when testing the server interactively, start the server in the `webTests` directory after the program has been compiled.

```bash
cd HTTPServer/src/webTests
```

Ensure `webTests` contains the following files and structure:

```bash
webTests/
├── index.html
├── mimeTypeSamples/
│   ├── image.png
│   ├── image.jpeg
│   ├── image.gif
│   ├── image.ico
│   ├── text.html
│   ├── text.txt
│   ├── style.css
│   ├── script.js
│   └── data.json
└── postBin/
    └── postBin.txt
```

## Basic Server Functionality

| Command | Expected Output | Reason for Test |
| ------- | --------------- | --------------- |
| `curl 10.65.255.109:8080` | Contents of `index.html` | Test if the server correctly serves the default page. |
| `curl 10.65.255.109:8080/nonexistent` | Contents of `404.html` | Test if the server correctly serves the 404 error page for a nonexistent route. |
| `curl 10.65.255.109:8080/ ` | List of files in `webTests` | Test if the server correctly serves a list of files in the directory. |
| `curl 10.65.255.109:8080/mimeTypeSamples/image.png` | Binary data of `image.png` | Test if the server correctly serves an image file. |

repeated for each file in `mimeTypeSamples`

## Server Functionality with HTTP Methods

| Command | Expected Output | Reason for Test |
| ------- | --------------- | --------------- |
| `curl -X GET 10.65.255.109:8080` | Contents of `index.html` | Test if the server correctly handles a GET request. |
| `curl -X POST -d "data" 10.65.255.109:8080/postBin/postBin.txt` | Response for a POST request | Test if the server correctly handles a POST request and saves the data to `postBin.txt`. |
| `curl -X POST -d @mimeTypeSamples/text.txt 10.65.255.109:8080/postBin/postBin.txt` | Response for a POST request | Test if the server correctly handles a POST request and saves the data to `postBin.txt`. |

## Server Functionality with Concurrent Connections

| Command | Expected Output | Reason for Test |
| ------- | --------------- | --------------- |
| `curl 10.65.255.109:8080 & curl 10.65.255.109:8080/mimeTypeSamples/image.png` | Contents of `index.html` and binary data of `image.png` | Test if the server correctly handles multiple concurrent connections. |

## Server Functionality with multiple clients

Have a friend or 5 connect to the server and test the functionality of the server. Ensure that the server is able to handle multiple clients at once.

## Load Testing

Use Apache Benchmark to test the server's performance under load.

```bash
ab -n 50 -c 50 -s 120 http://10.65.255.109:8080/
```



