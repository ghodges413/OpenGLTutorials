//
//  Client.cpp
//
#include "Sockets.h"
#include "Client.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Mswsock.lib" )
#pragma comment( lib, "AdvApi32.lib" )

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "666"

#define BUFSIZE 512

/*
===============================
Client
===============================
*/
class Client {
public:
    Client() {}
    ~Client() {}

    WSADATA m_wsaData;
    SOCKET m_connectSocket = INVALID_SOCKET;
    SOCKET m_udpSocket = INVALID_SOCKET;

    struct addrinfo m_hints;

    char m_recvbuf[ DEFAULT_BUFLEN ];
    int m_recvbuflen = DEFAULT_BUFLEN;

    // UDP
    bool InitUDP();
    bool SendTo( char * buffer, int bufferLength );
    bool SendTo( char * buffer, int bufferLength, char * address, int port );
    void CloseUDP();

    // TCP
    bool Connect( const char * ipaddress );
    bool Send( const char * data, int datalength );
    bool Receive();
    void Shutdown();
};

/*
===============================
Client::InitUDP
===============================
*/
bool Client::InitUDP() {
	// Initialize Winsock
	int result = WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
	if ( 0 != result ) {
		printf( "WSAStartup failed with error: %i\n", result );
		return false;
	}
	
	// Create a udp socket
    m_udpSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( SOCKET_ERROR == m_udpSocket ) {
		printf( "Could not create socket : %d" , WSAGetLastError() );
        return false;
	}
}

/*
===============================
Client::SendTo
===============================
*/
bool Client::SendTo( char * buffer, int bufferLength ) {
    sockaddr_in si_other;
    int slen = sizeof( si_other );    

    // Setup address structure
	memset( &si_other, 0, sizeof( si_other ) );
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons( 666 );
	si_other.sin_addr.S_un.S_addr = inet_addr( "127.0.0.1" );

	// Send to
    int bytesSent = sendto( m_udpSocket, buffer, strlen( buffer ), 0, (sockaddr *)&si_other, slen );
	if ( SOCKET_ERROR == bytesSent ) {
		printf( "sendto failed with error: %ld\n", WSAGetLastError() );
		return false;
	}

	memset( buffer, 0, bufferLength );

	// Receive from, this is a block call
    int bytesReceived = recvfrom( m_udpSocket, buffer, bufferLength, 0, (sockaddr *)&si_other, &slen );
	if ( SOCKET_ERROR == bytesReceived ) {
		printf( "recvfrom failed with error: %ld\n", WSAGetLastError() );
		return false;
	}

    return true;
}

/*
===============================
Client::SendTo
===============================
*/
bool Client::SendTo( char * buffer, int bufferLength, char * address, int port ) {
    sockaddr_in si_other;
    int slen = sizeof( si_other );

    // Setup address structure
	memset( &si_other, 0, sizeof( si_other ) );
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons( port );
	si_other.sin_addr.S_un.S_addr = inet_addr( address );

	// Send to
    int bytesSent = sendto( m_udpSocket, buffer, strlen( buffer ), 0, (sockaddr *)&si_other, slen );
	if ( SOCKET_ERROR == bytesSent ) {
		printf( "sendto failed with error: %ld\n", WSAGetLastError() );
		return false;
	}

    return true;
}

/*
===============================
Client
===============================
*/
void Client::CloseUDP() {
    closesocket( m_udpSocket );
	WSACleanup();
}

/*
===============================
Client::Connect
===============================
*/
bool Client::Connect( const char * ipaddress ) {
    // Initialize Winsock
    int iResult = WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
    if ( 0 != iResult ) {
        printf( "WSAStartup failed with error: %i\n", iResult );
        return false;
    }

    ZeroMemory( &m_hints, sizeof( m_hints ) );
    m_hints.ai_family = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    struct addrinfo * addresses = NULL;
    iResult = getaddrinfo( ipaddress, DEFAULT_PORT, &m_hints, &addresses );
    if ( 0 != iResult ) {
        printf( "getaddrinfo failed with error: %i\n", iResult );
        WSACleanup();
        return false;
    }

    // Attempt to connect to an address until one succeeds
    struct addrinfo * ptr = NULL;
    for ( ptr = addresses; ptr != NULL; ptr = ptr->ai_next ) {
        // Create a SOCKET for connecting to server
        m_connectSocket = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol );
        if ( INVALID_SOCKET == m_connectSocket ) {
            printf( "socket failed with error: %ld\n", WSAGetLastError() );
            WSACleanup();
            return false;
        }

        // Connect to server.
        iResult = connect( m_connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen );
        if ( SOCKET_ERROR == iResult ) {
            closesocket( m_connectSocket );
            m_connectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo( addresses );

    if ( INVALID_SOCKET == m_connectSocket ) {
        printf( "Unable to connect to server!\n" );
        WSACleanup();
        return false;
    }

    return true;
}

/*
===============================
Client::Send
===============================
*/
bool Client::Send( const char * data, int datalength ) {
    // Send an initial buffer
    int bytesSent = send( m_connectSocket, data, datalength, 0 );
    if ( SOCKET_ERROR == bytesSent ) {
        printf( "send failed with error: %i\n", WSAGetLastError() );
        closesocket( m_connectSocket );
        WSACleanup();
        return false;
    }

    printf( "Bytes Sent: %ld\n", bytesSent );
    return true;
}

/*
===============================
Client::Receive
===============================
*/
bool Client::Receive() {
    // This is a blocking call
    int bytesReceived = recv( m_connectSocket, m_recvbuf, m_recvbuflen - 1, 0 );

    if ( bytesReceived > 0 ) {
        printf( "Bytes received: %i\n", bytesReceived );
        m_recvbuf[ bytesReceived + 1 ] = 0;
        printf( "Received Buffer: %s\n", m_recvbuf );
    }
    if ( 0 == bytesReceived ) {
        printf( "Connection closed\n" );
    }
    if ( bytesReceived < 0 ) {
        printf( "recv failed with error: %i\n", WSAGetLastError() );
    }

    return ( bytesReceived >= 0 );
}

/*
===============================
Client::Shutdown
===============================
*/
void Client::Shutdown() {
    // shutdown the connection since no more data will be sent
    int result = shutdown( m_connectSocket, SD_SEND ); // This sends a termination to the server?
    if ( SOCKET_ERROR == result ) {
        printf( "shutdown failed with error: %i\n", WSAGetLastError() );
    }

    // cleanup
    closesocket( m_connectSocket );
    WSACleanup();
}

/*
===============================
TestClient
===============================
*/
void TestClient() {
    Client client;
    client.Connect( "127.0.0.1" );

    const char * message = "Client Test Message";
    client.Send( message, strlen( message ) );
    client.Receive();
    client.Shutdown();
}








#include "Threading/Threads.h"
#include "Threading/Mutex.h"
#include "Threading/ThreadLocks.h"

static Mutex * s_clientMutex = NULL;
static Thread s_clientThread;

static char s_data[ 512 ];
static int s_size = 0;

static Client s_client;

/*
================================
InitClient
================================
*/
void InitClient() {
#if defined( USE_UDP )
    s_client.InitUDP();
#else
    s_client.Connect( "127.0.0.1" );
#endif
}

/*
================================
ClientSendThread
================================
*/
ThreadReturnType_t ClientSendThread( ThreadInputType_t data ) {
    InitClient();

	while ( 1 ) {
        ScopedLock( *s_clientMutex );

        if ( s_size > 0 ) {
#if defined( USE_UDP )
            s_client.SendTo( s_data, s_size, "127.0.0.1", 666 );
#else
            s_client.Send( s_data, s_size );
#endif
            s_size = 0;
        }

        // TODO: Convert the buffer into the player's position and view direction
        // This should be done via mutex to make the update "safe".
        // Although it should also be done when the physics isn't running.
        // Otherwise we open ourselves to weird collision/resolve bugs.
        // Which means we need to be able to set such things in a deferred sort of way.
        // But for our needs it'll be okay to just set the position and view direction.
	}

    s_client.Shutdown();
	return NULL;
}

/*
================================
InitClientThread
================================
*/
void InitClientThread() {
	s_clientMutex = new Mutex;
	s_clientThread.Create( ClientSendThread, NULL );
}

/*
================================
EndClientThread
================================
*/
void EndClientThread() {
#if defined( USE_UDP )
    s_client.CloseUDP();
#else
    s_client.Shutdown();
#endif
	s_clientThread.Join();

	delete s_clientMutex;
	s_clientMutex = NULL;
}

/*
================================
ClientSendViewPos
================================
*/
void ClientSendViewPos( float theta, float phi, Vec3 pos ) {
    ScopedLock( *s_clientMutex );

    if ( s_size > 0 ) {
        return;
    }

    // Pack view and pos into a buffer
    char * dataPtr = s_data;

    int size = 0;
    memcpy( dataPtr + size, &theta, sizeof( float ) );
    size += sizeof( float );
    memcpy( dataPtr + size, &phi, sizeof( float ) );
    size += sizeof( float );
    memcpy( dataPtr + size, pos.ToPtr(), sizeof( Vec3 ) );
    size += sizeof( Vec3 );

    s_size = size;
}