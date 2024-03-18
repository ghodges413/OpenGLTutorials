//
//  Sockets.cpp
//
#include "Sockets.h"
#include "Client.h"
#include "Server.h"

#if defined( USE_POSIX )
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Mswsock.lib" )
#pragma comment( lib, "AdvApi32.lib" )
#endif

#if defined( USE_POSIX )
typedef int SOCKET_T;
#else
typedef SOCKET SOCKET_T;
typedef unsigned short in_port_t;
#endif

/*
===============================
SocketSend
===============================
*/
int SocketSend( SOCKET_T sock, char * byteBuffer, int bufferLength, int flags ) {
    return send( sock, byteBuffer, bufferLength, flags );
}

/*
===============================
SocketRecv
===============================
*/
int SocketRecv( SOCKET_T sock, char * byteBuffer, int bufferLength, int flags ) {
    return recv( sock, byteBuffer, bufferLength, flags );
}

/*
===============================
SocketListen
===============================
*/
int SocketListen( SOCKET_T sock, int backlog ) {
    return listen( sock, backlog );
}

/*
===============================
SocketConnect
===============================
*/
int SocketConnect( SOCKET_T sock, void * servAddr, int len ) {
    return connect( sock, (struct sockaddr *)servAddr, len );
}

/*
===============================
SocketBind
===============================
*/
int SocketBind( SOCKET_T sock, void * servAddr, int servLength ) {
    return bind( sock, (struct sockaddr *)servAddr, servLength ) ;
}

/*
===============================
SocketSocket
===============================
*/
SOCKET_T SocketSocket( int family, int type, int protocol ) {
    return socket( family, type, protocol );
}

/*
===============================
SocketClose
===============================
*/
void SocketClose( SOCKET_T sock ) {
#if defined( USE_POSIX )
    close( sock );
#else
    closesocket( sock );
#endif
}

/*
===============================
SocketGetAddrInfo
===============================
*/
int SocketGetAddrInfo( char * address, char * port, addrinfo * hints, addrinfo ** results ) {
    return getaddrinfo( address, port, hints, results );
}

/*
===============================
SocketShutdown
===============================
*/
int SocketShutdown( SOCKET_T sock, int how ) {
    return shutdown( sock, how );
}

/*
===============================
SocketAccept
===============================
*/
int SocketAccept( SOCKET_T sock, sockaddr * address, int * addressLength ) {
    return accept( sock, address, addressLength );
}

// Network to Host byte ordering
unsigned short SocketNtoHS( unsigned short s ) {
    return ntohs( s );
}

// Host to Network byte ordering
unsigned short SocketHtoNS( unsigned short s ) {
    return htons( s );
}

// Converts from printable address to binary address
int SocketINet_PtoN( int family, char * address, void * addressBuffer ) {
    return inet_pton( family, address, addressBuffer );
}

// Converts from binary address to printable address
const char * SocketINet_NtoP( int family, const void * address, char * addressBuffer, int bufferSize ) {
    return inet_ntop( family, address, addressBuffer, bufferSize );
}

/*
===============================
PrintSocketAddress
===============================
*/
void PrintSocketAddress( const sockaddr * address ) {
    if ( NULL == address ) {
        return;
    }

    void * numericAddress; // pointer to binary address
    char addrBuff[ INET6_ADDRSTRLEN ]; // buffer to contain resulst (ipv6 sufficient tho hold ipv4)
    in_port_t port; // port to print

    switch ( address->sa_family ) {
        case AF_INET: {
            numericAddress = &((sockaddr_in *)address)->sin_addr;
            port = ntohs( ((sockaddr_in *)address)->sin_port );
        } break;
        case AF_INET6: {
            numericAddress = &((sockaddr_in6 *)address)->sin6_addr;
            port = ntohs( ((sockaddr_in6 *)address)->sin6_port );
        } break;
        default: return;
    }

    // convert binary to printable address
    if ( NULL == inet_ntop( address->sa_family, numericAddress, addrBuff, sizeof( addrBuff ) ) ) {
        printf( "invalid address" );
    } else {
        printf( "%s", addrBuff );
        if ( 0 != port ) {
            printf( "-%u", port );
        }
    }
}

/*
===============================
PrintGetAddrInfo
===============================
*/
int PrintGetAddrInfo( char * address, char * port ) {
    // tell the system what kinds of address info we want
    addrinfo addrCriteria;
    memset( & addrCriteria, 0, sizeof( addrCriteria ) );
    addrCriteria.ai_family = AF_UNSPEC;     // any address family
    addrCriteria.ai_socktype = SOCK_STREAM; // only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP; // only tcp protocol

    // get addresses associated wit the specified name/service
    addrinfo * addrList;

    // modify servaddr contents to reference linked list of addresses
    int rtnVal = getaddrinfo( address, port, &addrCriteria, &addrList );
    if ( 0 != rtnVal ) {
        return rtnVal;
    }

    // display returned addresses
    for ( struct addrinfo * addr = addrList; addr != NULL; addr->ai_next ) {
        PrintSocketAddress( addr->ai_addr );
    }

    freeaddrinfo( addrList );
    return 0;
}