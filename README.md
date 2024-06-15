# stream-connection
Implement gPRC bidirectional streaming server and client in Minikube with Ingress and TLS.

## Prerequisites
- install [Docker Desktop](https://www.docker.com/products/docker-desktop/) and [Minikube](https://minikube.sigs.k8s.io/docs/start/?arch=%2Fmacos%2Fx86-64%2Fstable%2Fbinary+download#Ingress)
- enable Ingress and [registry](https://minikube.sigs.k8s.io/docs/handbook/registry/) addon in Minikube

## Usage
### Create a selt-signed certificate and kubectl secret TLS
For simplicity, only one certificate is created and used in this project. 
```
openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -keyout tls.key -out tls.crt -subj "/CN=grpctest.gary.com/O=gary" -addext "subjectAltName=DNS:grpctest.gary.com"
kubectl create secret tls grpc-test-secret --namespace grpc-test --key tls.key --cert tls
```
Then, modify `/etc/hosts` for DNS setting.
### Run your server and client
```
docker build -f Dockerfile.streamconnection -t streamconnection .
docker build -f Dockerfile.streamserver -t localhost:5000/streamserver .
docker push localhost:5000/streamserver
kubectl apply -f streamserver-ingress-tls.yaml
docker build -f Dockerfile.streamclient -t streamclient .
docker run --rm --network=host streamclient /app/bin/client grpctest.gary.com:443
```
