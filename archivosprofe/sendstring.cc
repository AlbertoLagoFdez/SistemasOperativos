#include <iostream>
#include <string>

#include <sys/socket.h>
#include <sys/types.h>
// #include <netinet/in.h>
// #include <netinet/ip.h>
#include <arpa/inet.h> 
#include <unistd.h>

// final que no me sale.
// g++ sendstring.cc -o -std=c++2b  sendstring
// #include <experimental/scope>
// using std::experimental::scope_exit;
// auto guar=scope_exit([fd_socket] { close(fd_socket)});

// std::error_code [usarlo]
// std::optional
// std::expected

int main(int argc, char **argv)
{
  int fd_socket=socket(AF_INET, SOCK_DGRAM, 0);
  if(fd_socket<0) 
  {
    std::cerr << "Error socket" << std::endl;
    return EXIT_FAILURE;
  }
  // Direccion de destino
  sockaddr_in remote_address {};
  remote_address.sin_family=AF_INET;
  remote_address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  remote_address.sin_port=htons(8080);
  std::string message_text("Hola, mundo!\n");

  int bytes_sent=sendto(fd_socket, 
                        message_text.data(), 
                        message_text.size(), 
                        0,
                        reinterpret_cast<const sockaddr*>(&remote_address),
                        sizeof(remote_address));
  if(bytes_sent<0)
  {
    std::cerr << "Error send" << std::endl;
    return EXIT_FAILURE;
  }
  close(fd_socket);
  std::cout << "Terminado" << std::endl;
  return EXIT_SUCCESS;
}