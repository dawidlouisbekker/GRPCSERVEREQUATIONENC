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
#include "grpcServer.h"



int main(int argc, char** argv) {
       
    int port = 0;
    bool isFinished = false;
    bool isConsoleFinished = false;

    port = StartHttpServer(isFinished, port);

    std::string input = "";

    RunServer(std::to_string(port));

    while (input != "exit") {
        
        std::getline(std::cin, input);
        if (input == "exit") {
            isConsoleFinished = true;
        }
    }

    return 0;
}


   