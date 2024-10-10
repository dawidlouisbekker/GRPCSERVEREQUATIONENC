#ifndef WEBSITE
#define WEBSITE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <string>
//rate limiting
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <mutex>
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include "grpcServer.h"
#include "equations.h"

//#include "PacketHandler.h"

#pragma comment(lib, "winhttp.lib")


double expectedWebAns;
int serverport;
int newServerPort;



std::string GenerateRandomBinaryStringWeb() {
    // Define the possible binary strings
    const std::string options[] = { "00", "01", "10", "11" };

    // Create a random number generator
    std::random_device rd; // Random device to seed the generator
    std::mt19937 gen(rd()); // Mersenne Twister generator
    std::uniform_int_distribution<> dist(0, 3); // Distribution for indices 0 to 3

    // Generate a random index and return the corresponding string
    return options[dist(gen)];
}

class RateLimiter {
public:
    RateLimiter(int maxRequests, std::chrono::seconds interval)
        : maxRequests(maxRequests), interval(interval), requestCount(0) {}

    bool allowRequest() {
        std::lock_guard<std::mutex> lock(mutex);
        auto now = std::chrono::steady_clock::now();

        // Reset the counter if the interval has passed
        if (now - lastReset >= interval) {
            lastReset = now;
            requestCount = 0;
        }

        if (requestCount < maxRequests) {
            requestCount++;
            return true; // Allow the request
        }

        return false; // Rate limit exceeded
    }

private:
    int maxRequests;
    std::chrono::seconds interval;
    int requestCount;
    std::chrono::steady_clock::time_point lastReset = std::chrono::steady_clock::now();
    std::mutex mutex;
};

std::string GetClientIP(const std::string& clientIP) {
    // Convert the port from network byte order to host byte order

    // Create a full address string
    std::string clientAddress = clientIP;
    return clientAddress;
    // Output the result

}

int GetClientPort(const sockaddr_in& clientAddr) {
    // Convert the port from network byte order to host byte order
    
    int clientPort = ntohs(clientAddr.sin_port);
    // Create a full address string
    return clientPort;
    // Output the result

}

void GenRandomIntWeb(INT32& a, INT32& b, INT32& c, INT32& x) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100); // Ch
    a = dis(gen);
    b = dis(gen);
    c = dis(gen);
    x = dis(gen);
}

int StartHttpServer(bool& isFinished, int& grpcPort) {
    WSADATA wsaData;
    SOCKET listeningSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);

    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 0;
    }

    // Create a socket
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 0;
    }

    // Set up the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    // Bind the socket
    if (bind(listeningSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(listeningSocket);
        WSACleanup();
        return 0;
    }

    // Listen for incoming connections
    listen(listeningSocket, SOMAXCONN);

    std::cout << "Server is listening on port 8080..." << std::endl;

    // Accept a client connection
    while (isFinished == false) {
        clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue; // Continue to accept new connections
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

        std::string ClientsIP  = GetClientIP(clientIP);
        int ClientPort = GetClientPort(clientAddr);
        char buffer[1024];
        int bytesReceived = 0;

        // Receive data in a loop
        while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0)) > 0) {          //-1
           
            buffer[bytesReceived] = '\0'; // Null-terminate the string
            std::cout << buffer; // Print the response
            std::cout << std::endl << "Bytes recieved:" << bytesReceived << std::endl;

            std::cout << "Received:" << std::string(buffer, bytesReceived) << std::endl;

            std::string EquationContext;
            // Optionally handle partial HTTP requests here
            INT32 a = 0;
            INT32 b = 0;
            INT32 c = 0;
            INT32 x = 0;
            Equations WebCalc = Equations();
            double expectedAns;
            do {
                GenRandomIntWeb(a, b, c, x);
                EquationContext = GenerateRandomBinaryStringWeb();
                expectedAns = WebCalc.CalcAnswer(a, b, c, x, EquationContext);

            } while (expectedAns < 0);
            
          //  int numbers[4] = { a,b,c,x };
          //  send(clientSocket, numbers, sizeof(numbers), 0);

            std::cout << "A: " << a << std::endl;
            std::cout << "B: " << b << std::endl;
            std::cout << "C: " << c << std::endl;
            std::cout << "X: " << x << std::endl;
            std::cout << EquationContext << std::endl;

           // valread = read( clientSocket, buffer, 1024);
           // int sum = atoi(buffer);
            // Example response
            const char* headers = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
                

            // Must be changed
            std::string body = 
                "A: " + std::to_string(a) + "\r\n"
                "B: " + std::to_string(b) + "\r\n"
                "C: " + std::to_string(c) + "\r\n"
                "X: " + std::to_string(x) + "\r\n"
                "E: " + EquationContext +  "\r\n"
                "Connection: close\r\n\r\n";

            send(clientSocket, body.c_str(), body.length(), 0);

            std::cout << "Expected ans:" << expectedAns << std::endl;

            while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[bytesReceived] = '\0'; // Null-terminate the string
                if (std::to_string(expectedAns) == std::string(buffer, bytesReceived)) {
                    int grpcPort = ((int)WebCalc.CalcAnswer(a, b, c, x, EquationContext) % 65535);
                    ClientIPs[grpcPort].push_back(ClientsIP);
                    ClientPorts[grpcPort].push_back(std::to_string(ClientPort));
                    ServerPorts.push_back(std::to_string(grpcPort));
                    return grpcPort;
                }
                //send(clientSocket, buffer, bytesReceived, 0);
                closesocket(clientSocket);
            }

            

            WSACleanup();
            
        }

        isFinished = true;
    }

    closesocket(listeningSocket);
    WSACleanup();
}

#endif

