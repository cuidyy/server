# 用户图片远程安全存储项目服务端

# 生成CA私钥
openssl genrsa -out ca.key 2048
 
# 生成CA自签名证书
openssl req -x509 -new -nodes -key ca.key -sha256 -days 365 -out ca.crt -subj "/CN=MyCA"

# 生成服务器私钥
openssl genrsa -out server.key 2048
 
# 生成服务器CSR
openssl req -new -key server.key -out server.csr -subj "/CN=server_ip"

# 使用CA签署服务器证书
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256

# 生成程序
mkdir build

cd build

cmake ..

make