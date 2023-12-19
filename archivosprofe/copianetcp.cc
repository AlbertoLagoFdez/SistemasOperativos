#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <expected>
#include <format>
#include <cstdint>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <system_error>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

/**
 *  === PARTE 2 ===
 * receive
 * socket -> fd_socket -> bind() 
 * IP + Port -> fd_socket
 * 
 * crear variables de entorno
 * export NETCP_IP
 * export NETCP_PORT
 * 
 * std::getnv (cppreference.com)
 * 
 * signals.c
 * 
**/

using open_file_result = std::expected<int, std::error_code>;
using make_socket_result = std::expected<int, std::error_code>;

[[nodiscard]]
open_file_result open_file(const std::string& path,
    int flags, mode_t mode)
{
  int fd = open(path.c_str(), flags, mode);
  if (fd == -1) 
  {
    std::error_code error(errno, std::system_category());
    return std::unexpected(error);
  }
  return fd;
}

[[nodiscard]]
std::error_code read_file(int fd, std::vector<uint8_t>& buffer)
{
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
  if (bytes_read < 0)
  {
    return std::error_code(errno, std::system_category());
  }
  buffer.resize(bytes_read);
  return std::error_code(0, std::system_category());
}

[[nodiscard]]
std::optional<sockaddr_in> make_ip_address(
    const std::optional<std::string> ip_address, uint16_t port=0)
{
  sockaddr_in remote_address {};
  remote_address.sin_family=AF_INET;
  remote_address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  remote_address.sin_port=htons(port);

  return remote_address;
}


make_socket_result make_socket(
    std::optional<sockaddr_in> address = std::nullopt)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
  {
    std::error_code error(errno, std::system_category());
    return std::unexpected(error);
  }
  return fd;
}


std::error_code send_to(int fd, const std::vector<uint8_t>& message,
     const sockaddr_in& address)
{
  int bytes_sent=sendto(fd,
                        message.data(),
                        message.size(),
                        0,
                        reinterpret_cast<const sockaddr*>(&address),
                        sizeof(address));
  if (bytes_sent<0)
  {
    return std::error_code(errno, std::system_category());
  }
  close(fd);
  return std::error_code(0, std::system_category());
}

// Imprimir el mensaje y la direccion remitente
/*
std::cout << std::format("El sistema '{}' envió el mensaje '{}'\n",
    ip_address_to_string(remote_address),
    message.text.c_str()
)
*/

//Segunda parte

std::error_code netcp_send_file(const std::string& filename) {
  
}

std::error_code netcp_receive_file(const std::string& filename);

struct program_options
{
  bool show_help = false;
  std::string input_file;     //Este seria el mensaje
  std::string output_filename;
//  std::string port;
  // ...
};

std::string getenv(const std::string& name) {
  char* value = getenv(name.c_str());
  if (value) {
    return std::string(value);
  }
  else {
    return std::string();
  }
}

void Usage() {
  std::cout << "./netcp [-h] ORIGEN" << std::endl;
}



std::optional<program_options> parse_args(int argc, char* argv[])
{
  if (argc < 2) return std::nullopt;
  std::vector<std::string_view> args(argv + 1, argv + argc);
  program_options options;
  for (auto it = args.begin(), end = args.end(); it != end; ++it)
  {
    if (*it == "-h" || *it == "--help")
    {
      options.show_help = true;
    }
    if (*it == "-l" || *it == "--listening")
    {
      if (++it != end)
      {
        options.output_filename = *it;
      }
      else
      {
      Usage();
      return std::nullopt;
      }
    }
    else
    {
      options.input_file = *it;
      //++it;
      //options.port = *it;
    }
  }
return options;
}



int main(int argc, char **argv) {
  auto options = parse_args(argc, argv);
  if (!options)
  {
    std::cerr << "Error!" << std::endl;
    return EXIT_FAILURE;
  }
// Usar options.value() para acceder a las opciones...
  if (options.value().show_help) 
  {
    Usage();  
  }

  int flags = O_RDONLY | O_CREAT;
  mode_t mode = 0666;
  open_file_result result = open_file(options.value().input_file, flags, mode);
  
  if (!result) {
    //return result.error();
    std::cerr << "¡Error! No se puede abrir el archivo." << std::endl;
    return EXIT_FAILURE;
  }

  int fd = *result;

  std::vector<uint8_t> buffer(1024);
  std::error_code error_read_file = read_file(fd, buffer);
  if (error_read_file)
  {
    std::cerr << std::format("Error ({}): {}\n", error_read_file.value(),
    error_read_file.message());
  }
  //int port = std::stoi(options.value().port);
  int sock_fd;
  auto address = make_ip_address("127.0.0.1", 8080);
  auto result_socket = make_socket(address.value());
  if (result_socket)
  {
    sock_fd = *result_socket;
  }

  std::error_code error_send_to = send_to(sock_fd, buffer, address.value());
  if (error_send_to)
  {
    std::cerr << std::format("Error ({}): {}\n", error_send_to.value(),
    error_send_to.message());
  }

  return EXIT_SUCCESS;
}