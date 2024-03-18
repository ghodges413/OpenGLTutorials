//
//  Server.cpp
//
#include "Server.h"
#include "Sockets.h"

//#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment( lib, "Ws2_32.lib" )

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "666"

/*
===============================
Server
===============================
*/
class Server {
public:
    Server() {}
    ~Server() {}

    WSADATA m_wsaData;

    SOCKET m_listenSocket = INVALID_SOCKET;
    SOCKET m_clientSocket = INVALID_SOCKET;
    SOCKET m_udpSocket = INVALID_SOCKET;

    addrinfo * m_result = NULL;
    addrinfo m_hints;

    char m_recvbuf[ DEFAULT_BUFLEN ];
    int m_recvbuflen = DEFAULT_BUFLEN;

    // UDP
    bool InitUDP();
    bool ReceiveFrom( char * buffer, int bufferLength );
    void CloseUDP();

    // TCP
    bool InitTCP();
    bool WaitForClient();
    bool Send( const char * data, int datalength );
    bool Receive();
    void Shutdown();
};

/*
===============================
Server::InitUDP
===============================
*/
bool Server::InitUDP() {
 	sockaddr_in server;
	
	// Initialize Winsock
	int result = WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
	if ( 0 != result ) {
        printf( "WSAStartup failed with error: %i\n", result );
        return false;
    }
	
	// Create a udp socket
    m_udpSocket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( INVALID_SOCKET == m_udpSocket ) {
		printf( "Could not create socket : %d" , WSAGetLastError() );
        return false;
	}
	
	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 666 ); // Set default port
	
	// Bind the udp socket
    result = bind( m_udpSocket, (sockaddr *)&server , sizeof( server ) );
	if ( SOCKET_ERROR == result ) {
		printf( "Bind failed with error code : %d" , WSAGetLastError() );
		return false;
	}

    return true;
}

/*
===============================
Server::ReceiveFrom
===============================
*/
bool Server::ReceiveFrom( char * buffer, int bufferLength ) {
    sockaddr_in si_other;
    int slen = sizeof( si_other );

    // Receive from, this is a block call
    int bytesReceived = recvfrom( m_udpSocket, buffer, bufferLength, 0, (sockaddr *)&si_other, &slen );
	if ( SOCKET_ERROR == bytesReceived ) {
		printf( "recvfrom failed with error: %ld\n", WSAGetLastError() );
		return false;
	}
		
	// print details of the client/peer and the data received
	printf( "Received packet from %s:%d\n", inet_ntoa( si_other.sin_addr ), ntohs( si_other.sin_port ) );
	printf( "Data: %s\n", buffer );
		
	// Echo the buffer back to the source
    int bytesSent = sendto( m_udpSocket, buffer, bufferLength, 0, (sockaddr *)&si_other, slen );
	if ( SOCKET_ERROR == bytesSent ) {
		printf( "sendto failed with error: %ld\n", WSAGetLastError() );
		return false;
	}

    return true;
}

/*
===============================
Server::CloseUDP
===============================
*/
void Server::CloseUDP() {
    closesocket( m_udpSocket );
	WSACleanup();
}

/*
===============================
Server::InitTCP
===============================
*/
bool Server::InitTCP() {
    // Initialize Winsock
    int result = WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
    if ( 0 != result ) {
        printf( "WSAStartup failed with error: %i\n", result );
        return false;
    }

    ZeroMemory( &m_hints, sizeof( m_hints ) );
    m_hints.ai_family = AF_INET;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_protocol = IPPROTO_TCP;
    m_hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    result = getaddrinfo( NULL, DEFAULT_PORT, &m_hints, &m_result );
    if ( 0 != result ) {
        printf( "getaddrinfo failed with error: %i\n", result );
        WSACleanup();
        return false;
    }

    // Create a SOCKET for the server to listen for client connections.
    m_listenSocket = socket( m_result->ai_family, m_result->ai_socktype, m_result->ai_protocol );
    if ( INVALID_SOCKET == m_listenSocket ) {
        printf( "socket failed with error: %ld\n", WSAGetLastError() );
        freeaddrinfo( m_result );
        WSACleanup();
        return false;
    }

    // Setup the TCP listening socket
    result = bind( m_listenSocket, m_result->ai_addr, (int)m_result->ai_addrlen );
    if ( SOCKET_ERROR == result ) {
        printf( "bind failed with error: %i\n", WSAGetLastError() );
        freeaddrinfo( m_result );
        closesocket( m_listenSocket );
        WSACleanup();
        return false;
    }

    freeaddrinfo( m_result );

    result = listen( m_listenSocket, SOMAXCONN );
    if ( SOCKET_ERROR == result ) {
        printf( "listen failed with error: %i\n", WSAGetLastError() );
        closesocket( m_listenSocket );
        WSACleanup();
        return false;
    }

    return true;
}

/*
===============================
Server::WaitForClient
===============================
*/
bool Server::WaitForClient() {
    // Accept a client socket
    m_clientSocket = accept( m_listenSocket, NULL, NULL );
    if ( INVALID_SOCKET == m_clientSocket ) {
        printf( "accept failed with error: %i\n", WSAGetLastError() );
        closesocket( m_listenSocket );
        WSACleanup();
        return false;
    }

    // No longer need server socket
    closesocket( m_listenSocket );
    return true;
}

/*
===============================
Server::Send
===============================
*/
bool Server::Send( const char * data, int datalength ) {
    // Send the data buffer
    int result = send( m_clientSocket, data, datalength, 0 );
    if ( SOCKET_ERROR == result ) {
        printf( "send failed with error: %i\n", WSAGetLastError() );
        closesocket( m_clientSocket );
        WSACleanup();
        return false;
    }

    printf( "Bytes Sent: %ld\n", result );
    return true;
}

/*
===============================
Server::Receive
===============================
*/
bool Server::Receive() {
    int bytesReceived = 0;

    // Receive until the peer shuts down the connection
    while ( bytesReceived >= 0 ) {
        int bytesReceived = recv( m_clientSocket, m_recvbuf, m_recvbuflen, 0 );

        if ( bytesReceived > 0 ) {
            printf( "Bytes received: %d\n", bytesReceived );
            m_recvbuf[ bytesReceived + 1 ] = 0;
            printf( "Received Buffer: %s\n", m_recvbuf );

            // Echo the buffer back to the sender
            int bytesSent = send( m_clientSocket, m_recvbuf, bytesReceived, 0 );
            if ( SOCKET_ERROR == bytesSent ) {
                printf( "send failed with error: %d\n", WSAGetLastError() );
                closesocket( m_clientSocket );
                WSACleanup();
                return false;
            }
            printf( "Bytes sent: %i\n", bytesSent );
        }

        if ( 0 == bytesReceived ) {
            printf( "Connection closing...\n" );
        }

        if ( bytesReceived < 0 ) {
            printf( "recv failed with error: %d\n", WSAGetLastError() );
            closesocket( m_clientSocket );
            WSACleanup();
            return false;
        }
    }

    return ( bytesReceived >= 0 );
}

/*
===============================
Server::Shutdown
===============================
*/
void Server::Shutdown() {
    // shutdown the connection since we're done
    int result = shutdown( m_clientSocket, SD_SEND );
    if ( SOCKET_ERROR == result ) {
        printf( "shutdown failed with error: %d\n", WSAGetLastError() );
    }

    // cleanup
    closesocket( m_clientSocket );
    WSACleanup();
}

/*
===============================
TestServer
===============================
*/
void TestServer() {
    Server server;
    server.InitTCP();
    server.WaitForClient();
    server.Receive();
    server.Shutdown();
}






#include "Threading/Threads.h"
#include "Entities/Player.h"

static Thread s_serverThread;

char s_receiveBuffer[ 512 ];

Server s_server;

/*
================================
InitServer
================================
*/
void InitServer() {
#if defined( USE_UDP )
    s_server.InitUDP();
#else
    s_server.InitTCP();
    s_server.WaitForClient();
#endif
}

/*
================================
ServerReceiveThread
================================
*/
ThreadReturnType_t ServerReceiveThread( ThreadInputType_t data ) {
    InitServer();

	while ( 1 ) {
#if defined( USE_UDP )
        sockaddr_in si_other;
        int slen = sizeof( si_other );
        int bytesReceived = recvfrom( s_server.m_udpSocket, s_receiveBuffer, 512, 0, (sockaddr *)&si_other, &slen );
        if ( bytesReceived <= 0 ) {
            continue;
        }
#else
        int bytesReceived = recv( s_server.m_clientSocket, s_receiveBuffer, 512, 0 );
        if ( bytesReceived <= 0 ) {
            break;
        }
#endif
        extern Player g_player;

        float theta;
        float phi;
        Vec3 pos;
        char * dataPtr = s_receiveBuffer;
        memcpy( &theta, dataPtr, sizeof( float ) );
        dataPtr += sizeof( float );
        memcpy( &phi, dataPtr, sizeof( float ) );
        dataPtr += sizeof( float );
        memcpy( pos.ToPtr(), dataPtr, sizeof( Vec3 ) );
        dataPtr += sizeof( Vec3 );

        g_player.SetViewPos( theta, phi, pos );

        // TODO: Convert the buffer into the player's position and view direction
        // This should be done via mutex to make the update "safe".
        // Although it should also be done when the physics isn't running.
        // Otherwise we open ourselves to weird collision/resolve bugs.
        // Which means we need to be able to set such things in a deferred sort of way.
        // But for our needs it'll be okay to just set the position and view direction.
	}

#if !defined( USE_UDP )
    s_server.Shutdown();
#endif
	return NULL;
}

/*
================================
InitServerThread
================================
*/
void InitServerThread() {
	s_serverThread.Create( ServerReceiveThread, NULL );
}

/*
================================
EndServerThread
================================
*/
void EndServerThread() {
#if defined( USE_UDP )
    s_server.CloseUDP();
#else
    s_server.Shutdown();
#endif
	s_serverThread.Join();
}