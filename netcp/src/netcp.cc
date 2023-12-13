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

// crear archivo para enviar 
// dd if=/dev/urandom of=testfile bs=1k count=1 iflag=fullblock

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
#include <experimental/scope>

using open_file_result = std::expected<int, std::error_code>;
using make_socket_result = std::expected<int, std::error_code>;
using std::experimental::scope_exit;

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

std::optional<sockaddr_in> make_ip_address(
    const std::optional<std::string> ip_address, uint16_t port)
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
  size_t bytes_sent=sendto(fd,
                        message.data(),
                        message.size(),
                        0,
                        reinterpret_cast<const sockaddr*>(&address),
                        sizeof(address));
  if (bytes_sent<0)
  {
    return std::error_code(errno, std::system_category());
  }
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

std::error_code netcp_send_file(const std::string& filename) 
{
  std::cout << "Send mode...\n";
  //Leer variables de entorno
  //
  //ip
  std::optional<std::string> ip;
  char* char_ip = std::getenv("NETCP_IP");
  if(!char_ip)
  {
    ip = "127.0.0.1";
  }
  else
  {
    ip = char_ip;
  }
  //port
  //
  uint16_t port;
  char* char_port = std::getenv("NETCP_PORT");
  if(char_port == NULL)
  {
    port = 8080;
  }
  else
  {
    port = static_cast<uint16_t>(std::strtoul(char_port, nullptr, 10));
  }
  // ip address
  auto address = make_ip_address(ip, port);
  if (!address)
  {
    std::error_code error(errno, std::system_category());
    std::cerr << "Error make ip address.\n";
    return error;
  }

  // socket
  int sock_fd;
  auto result_socket = make_socket(address.value());
  if (result_socket)
  {
    sock_fd = result_socket.value();
  }
  else
  {
    std::error_code error(errno, std::system_category());
    std::cerr << "Error at make socket.\n";
    return error;
  }

  //abrir archivo
  //
  int flags = O_RDONLY;
  mode_t mode = 0666;
  
  open_file_result fd = open_file(filename, flags, mode);
  if (!fd.has_value())
  {
    std::cout << "Error open file\n";
    return std::error_code(fd.error().value(), fd.error().category());
  }
  //int fd = *result_openfile;

  auto src_guard=scope_exit(
            [sock_fd] {close(sock_fd); });

  auto src_guard2=scope_exit(
            [fd] {close(fd.value()); });

  // Enviar el menzaje
  std::vector<uint8_t> buffer(1024);
  while (buffer.size() > 0) 
  {
    std::cout << "paco";
    std::error_code error_read_file = read_file(fd.value(), buffer);
    // Error en el read file.
    //
    if (error_read_file)
    {
      close(fd.value());
      error_read_file.message();
      std::error_code error (error_read_file.value() ,error_read_file.category());
      std::cout << "Error read file\n";
      return error;
    }

    std::error_code error_send_to = send_to(sock_fd, buffer, address.value());
    //Error en el send_to.
    //
    if (error_send_to)
    {
      close(fd.value());
      error_send_to.message();
      std::error_code error (error_send_to.value(), error_send_to.category());
      std::cout << "Error send to\n";
      return error;
    }
    usleep(1000);
  }
  close(fd.value());
  close(sock_fd);
  std::cout << "ok...\n";
  return std::error_code(0, std::system_category());
}


/*
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
  if (bytes_read < 0)
  {
    return std::error_code(errno, std::system_category());
  }
  buffer.resize(bytes_read);
  return std::error_code(0, std::system_category());
*/
std::error_code write_file(int fd, std::vector<uint8_t>& buffer)
{
  ssize_t bytes_write = write(fd, buffer.data(), buffer.size());
  if(bytes_write < 0)
  {
    return std::error_code(errno, std::system_category());
  }
  return std::error_code(0, std::system_category());
}

// Funcion para recibir el mensaje con un std::String
std::error_code recieve_from(int fd, std::vector<uint8_t>& buffer,
    sockaddr_in& address)
{
  socklen_t src_len = sizeof(address);

  int bytes_recv = recvfrom(fd,
        buffer.data(), 
        buffer.size(), 
        0,
        reinterpret_cast<sockaddr*>(&address),
        &src_len);
  
  if(bytes_recv < 0)
  {
    return std::error_code(errno, std::system_category());
  }
  
  buffer.resize(bytes_recv);
  return std::error_code(0, std::system_category());
/*
  std::cout << std::format("El sistema '{}'' envió el mensaje '{}'\n",
    ip_address_to_string(address),
    message.text.c_str() );
*/
}

std::error_code netcp_recieve_file(const std::string& filename) 
{
  std::cout << "listen mode...\n";
  std::cout << "waiting...\n";
  //Leer variables de entorno
  //
  //ip
  std::optional<std::string> ip;
  char* char_ip = std::getenv("NETCP_IP");
  if(!char_ip)
  {
    ip = "0.0.0.0";
  }
  else
  {
    ip = char_ip;
  }
  //port
  //
  uint16_t port;
  char* char_port = std::getenv("NETCP_PORT");
  if(char_port == NULL)
  {
    port = 8080;
  }
  else
  {
    port = static_cast<uint16_t>(std::strtoul(char_port, nullptr, 10));
  }
  // ip address
  auto address = make_ip_address(ip, port);
  if (!address)
  {
    std::error_code error(errno, std::system_category());
    std::cerr << "Error make ip address.\n";
    return error;
  }

  // socket
  int sock_fd;
  auto result_socket = make_socket(address.value());
  if (result_socket)
  {
    sock_fd = result_socket.value();
  }
  else
  {
    std::error_code error(errno, std::system_category());
    std::cerr << "Error at make socket.\n";
    return error;
  }

  if (bind(sock_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
    std::cerr << "Error assign dir to socket.\n";
    close(sock_fd);
    std::error_code error(errno, std::system_category());
    return error;
  }

  //abrir archivo
  //
  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  mode_t mode = 0666;
  
  open_file_result fd = open_file(filename, flags, mode);
  if (!fd.has_value())
  {
    std::cout << "Error open file\n";
    return std::error_code(fd.error().value(), fd.error().category());
  }
  //int fd = *result_openfile;

  auto src_guard=scope_exit(
            [sock_fd] {close(sock_fd); });

  auto src_guard2=scope_exit(
            [fd] {close(fd.value()); });

  // Enviar el menzaje
  std::vector<uint8_t> buffer(1024);
  while (true) 
  {
    //receive
    std::error_code error_recieve = recieve_from(sock_fd, buffer, address.value());
    if (error_recieve)
    {
      std::cerr << "Error at recieve.\n";
      std::error_code error(errno, std::system_category());
      return error; 
    }

    if (!buffer.empty())
    {
      std::error_code error_write_file = write_file(fd.value(), buffer);
      if (error_write_file)
      {
        std::cerr << "Error writing file.\n";
        std::error_code error(errno, std::system_category());
        return error;
      }
    }
    else{
      break;
    }
  }
  close(fd.value());
  close(sock_fd);
  std::cout << "ok...\n";
  return std::error_code(0, std::system_category());
}

struct program_options
{
  bool show_help = false;
  std::string output_filename;
  bool listening_mode =  false;
  std::string input_file;     //Este seria el mensaje

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
        options.listening_mode = true;
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
  
  // Si listening_mode esta activado, entra en el modo de esucha;
  if (options.value().listening_mode)
  {
    //std::cout << "Entra en el modo de recibir mensaje." << std::endl;
    std::error_code error_receive = netcp_recieve_file(options.value().output_filename);
  }
  else 
  {
    std::error_code error_send = netcp_send_file(options.value().input_file);
  }
  return EXIT_SUCCESS;
}