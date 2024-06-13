#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "proto/streamconnection.grpc.pb.h"
#include "proto/streamconnection.pb.h"

constexpr char kRootCertFilePath[] = "/app/bin/tls.crt";

std::string ReadFile(const char* file_path) {
  std::fstream input_file(file_path);
  std::string content = "";
  content.assign(std::istreambuf_iterator<char>(input_file),
                 std::istreambuf_iterator<char>());
  input_file.close();
  return content;
}

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

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "One and only argument [server address:port] is needed."
              << std::endl;
    return 0;
  }
  std::string server_addr_port = argv[1];
  Client client(grpc::CreateChannel(
      server_addr_port, grpc::SslCredentials(grpc::SslCredentialsOptions{
                            .pem_root_certs = ReadFile(kRootCertFilePath)})));
  client.GetValues();
  return 0;
}
