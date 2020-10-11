# DNS Management Service

This is a client-server dns management service.

The clients query to the server to get information using sockets.

Thep server reads from a file to initialize the dns table.
Also, it talks to DNS Service and stores the IP in the dns table.
The DNS table is not stored in a file.


## Getting started

### Prerequisites

* make
* gcc

### Compilation

```
make
```

## Running

```
./nameServer -f dns.txt
```

In another terminal:

```
./nameClient
```
