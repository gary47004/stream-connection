#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <iostream>
#include <memory>
#include <string>

#include "proto/streamconnection.grpc.pb.h"
#include "proto/streamconnection.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using streamconnection::Request;
using streamconnection::Response;
using streamconnection::StreamConnection;

class StreamConnectionImpl final : public StreamConnection::Service {
 public:
  StreamConnectionImpl() {}

  Status GetValues(ServerContext* context,
                   ServerReaderWriter<Response, Request>* stream) override {
    Request req;
    while (stream->Read(&req)) {
      Response res;
      count_++;
      res.set_value(std::to_string(count_));
      stream->Write(res);
    }

    return Status::OK;
  }

 private:
  int count_ = 0;
};

int main() {
  std::string server_address("0.0.0.0:50051");
  StreamConnectionImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
  return 0;
}
