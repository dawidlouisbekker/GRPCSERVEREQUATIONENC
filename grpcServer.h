#pragma once
#ifndef GRPCSERVER
#define GRPCSERVER

#include <iostream>
#include <memory>
#include <string>
#include <random>
#include <set>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <google/protobuf/message.h>
#include "messages.grpc.pb.h"
#include "messages.pb.h"
#include "equations.h"
#include <Windows.h>
#include "ClientWebEntry.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReaderWriter;
using validator::Validator;
using validator::IP;
using validator::Answer;
using validator::Variables;
using validator::Recieved;
using validator::Port;

bool SwitchPort = false;

std::string server_address;

std::vector<std::string> ClientIPs[65535];
std::vector<std::string> ClientPorts[65535];

std::vector<std::string> ServerPorts;
std::unique_ptr<grpc::Server> staticServer;

int rows = 0, cols = 0;
int gRPCPort = 0;

std::vector<std::vector<std::string>> ClientIPTable(rows, std::vector<std::string>(cols));

bool isValidIP(std::string clientAddress, int ServerPort) {
    for (int i = 0; ClientPorts[ServerPort].size() > i; i++) {
        int port = stoi(ClientPorts[ServerPort][i]);
        std::string expectedIP =  "ipv4:" + ClientIPs[ServerPort][i] + ':' + std::to_string(port + 1);
        std::cout << "Expected IP: " <<  expectedIP << std::endl;
        std::cout << "Client Addres: " << clientAddress << std::endl;
        if (clientAddress == expectedIP) {
            return true;
            break;
        }
    }
    //switch gRPC server and clients
    return false;
}

std::string GenerateRandomBinaryString() {
    // Define the possible binary strings
    const std::string options[] = { "00", "01", "10", "11" };

    // Create a random number generator
    std::random_device rd; // Random device to seed the generator
    std::mt19937 gen(rd()); // Mersenne Twister generator
    std::uniform_int_distribution<> dist(0, 3); // Distribution for indices 0 to 3

    // Generate a random index and return the corresponding string
    return options[dist(gen)];
}

void GenRandomInt(INT32& a, INT32& b, INT32& c, INT32& x) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100); // Ch
    a = dis(gen);
    b = dis(gen);
    c = dis(gen);
    x = dis(gen);
}

void ShutdownServer() {
    if (staticServer) {
        std::cout << "Shutting down the server..." << std::endl;
        staticServer->Shutdown(); // Initiates graceful shutdown
        staticServer.reset(); // Optionally reset the server pointer
        std::cout << "Server has been shut down." << std::endl;
    }
}

double last_answer_;
double expected_answer_;
std::set<std::string> blocked_ips_;

//bool IsIPexpected()

class ValidatorService final : public Validator::Validator::Service {



public:

    ValidatorService() {
    }


    Status EstablishCon(ServerContext* context, const IP* request, Variables* reply) override {

        INT32 a;
        INT32 b;
        INT32 c;
        INT32 x;

        GenRandomInt(a, b, c, x);
        std::string EContext = GenerateRandomBinaryString();
        reply->set_a(a);
        reply->set_b(b);
        reply->set_c(c);
        reply->set_x(x);
        reply->set_equtioncontext(EContext); // Set context if needed

        std::string client_ip = context->peer();
        if (isValidIP(client_ip, gRPCPort) == true) {
            std::cout << "Valid";
        }
        else {
            std::cout << "Invalid";
        }
        std::cout << "Received request from: " << client_ip << std::endl;
        Equations CalcServer = Equations();

        expected_answer_ = CalcServer.CalcAnswer(a, b, c, x, EContext);



        return grpc::Status::OK;
        return Status::OK;
    }



    Status GiveVariables(ServerContext* context, const Answer* request, Variables* reply) override {
        std::string client_ip = context->peer();
        double current_answer = request->answer();
        if (current_answer != expected_answer_) {
            blocked_ips_.insert(client_ip);

            //ip recording and switch port while transferring others to new port
            return grpc::Status::CANCELLED;
        }

        std::cout << "Recieved server ans: " << current_answer << std::endl;


        INT32 a;
        INT32 b;
        INT32 c;
        INT32 x;

        GenRandomInt(a, b, c, x);

        reply->set_a(a);
        reply->set_b(b);
        reply->set_c(c);
        reply->set_x(x);

        std::string EContext = GenerateRandomBinaryString();
        Equations CalcServer = Equations();
        expected_answer_ = CalcServer.CalcAnswer(a, b, c, x, EContext);

        // Optionally set an equation context (just an example)



        reply->set_equtioncontext(EContext);
        return grpc::Status::OK;
    }  

    // Implement NewPort for bidirectional streaming
    Status NewPort(ServerContext* context,
        ServerReaderWriter<Recieved, Port>* stream) override {
        Port port;
        while (stream->Read(&port)) {
            int port_number = port.port();

            // Process the port data, here we just send back a dummy response
            std::cout << "Received port: " << port_number << std::endl;

            Recieved response;
            response.set_ans(port_number + 1);  // Modify as per your logic

            // Send the response back to the client
            stream->Write(response);
        }
        return Status::OK;
    } 
};

void RunServerRandomPort(int max_retries, std::string& return_port) { // Change to 127.0.0.1:50051 if needed

    INT32 a;
    INT32 b;
    INT32 c;
    INT32 x;

    int port;

    ValidatorService service;

    ServerBuilder builder;

    int attempt = 0;
    while (attempt < max_retries) {
        GenRandomInt(a, b, c, x);
        Equations CalcServer = Equations();
        port = (int)CalcServer.CalcAnswer(a, b, c, x, GenerateRandomBinaryString()) % 65535;
        server_address = "127.0.0.1:" + std::to_string(port);
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        staticServer = builder.BuildAndStart();
        if (staticServer) {
            // Successfully started the server
            std::cout << "Server listening on " << server_address << std::endl;
            staticServer->Wait(); // Blocks until the server is shutdown
            return_port = std::to_string(port);     // Return the address where the server is running
        }
        else {
            std::cerr << "Failed to start server on attempt " << (attempt + 1) << std::endl;
            attempt++;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before retrying
        }
        staticServer->Wait();

    }
}

void RunServer(std::string Port) { // Change to 127.0.0.1:50051 if needed

    gRPCPort = stoi(Port);
    ValidatorService service;
    std::cout << "Detected port" << Port << std::endl;
    ServerBuilder builder;

    int attempt = 0;

        Equations CalcServer = Equations();
        //port = (int)CalcServer.CalcAnswer(a, b, c, x, GenerateRandomBinaryString()) % 65535;
        server_address = "127.0.0.1:" + Port;
        int selectedPort = 0;

        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<grpc::Server> server;
        server = builder.BuildAndStart();
        if (server) {
            // Successfully started the server
          std::cout << "Server listening on " << server_address << std::endl;
          // std::thread server_thread([&server]() {
          server->Wait();
            //    });
           // server_thread.detach(); // Detach the thread to allow it to run independently

        }   // Return the address where the server is running
        else {
            std::cerr << "Failed to start server on attempt " << (attempt + 1) << std::endl;
            attempt++;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before retrying
        }
  

}
#endif // !GRPCSERVER

