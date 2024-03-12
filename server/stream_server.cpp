#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <functional>
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
  StreamConnectionImpl(std::function<std::string()> generate_resp_msg)
      : generate_resp_msg_(generate_resp_msg) {}

  Status GetValues(ServerContext* context,
                   ServerReaderWriter<Response, Request>* stream) override {
    Request req;
    while (stream->Read(&req)) {
      Response res;
      res.set_value(generate_resp_msg_());
      stream->Write(res);
    }

    return Status::OK;
  }

 private:
  std::function<std::string()> generate_resp_msg_;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "One and only argument [server name] is needed." << std::endl;
    return 0;
  }
  std::string server_name = argv[1];
  int count = 0;
  auto generate_resp_msg = [&count, &server_name]() -> std::string {
    return "Server name: " + server_name +
           "\nMessage count: " + std::to_string(++count);
  };

  std::string server_address("0.0.0.0:50051");
  StreamConnectionImpl service(generate_resp_msg);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
  return 0;
}
