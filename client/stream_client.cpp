#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "proto/streamconnection.grpc.pb.h"
#include "proto/streamconnection.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using streamconnection::Request;
using streamconnection::Response;
using streamconnection::StreamConnection;

Request MakeRequest(const std::string& key) {
  Request req;
  req.set_key(key);
  return req;
}

class Client {
 public:
  explicit Client(std::shared_ptr<Channel> channel)
      : stub_(StreamConnection::NewStub(channel)) {}

  void GetValues() {
    ClientContext context;

    std::shared_ptr<ClientReaderWriter<Request, Response>> stream(
        stub_->GetValues(&context));

    std::thread writer([stream]() {
      std::vector<Request> requests{MakeRequest("first_key"),
                                    MakeRequest("second_key")};
      for (const Request& req : requests) {
        std::cout << "Sending key: " << req.key() << std::endl;
        stream->Write(req);
      }
      stream->WritesDone();
    });

    Response response;
    while (stream->Read(&response)) {
      std::cout << "Got value: " << response.value() << std::endl;
    }
    writer.join();
    Status status = stream->Finish();
    if (!status.ok()) {
      std::cout << "GetValues rpc failed." << std::endl;
    }
  }

 private:
  std::unique_ptr<StreamConnection::Stub> stub_;
};

int main() {
  Client client(grpc::CreateChannel("localhost:50051",
                                    grpc::InsecureChannelCredentials()));
  client.GetValues();
  return 0;
}
