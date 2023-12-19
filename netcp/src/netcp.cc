#include "netcp.h"

subprocess::subprocess(const std::vector<std::string> &args, subprocess::stdio redirected_io)
{
  std::cout << "System call subprocess constructor\n";
  args_ = args;
  redirected_io_ = redirected_io;
}

std::error_code subprocess::exec() 
{

}

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
    std::optional<sockaddr_in> address)
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
    std::error_code error(errno, std::system_category());
    return error;
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
    std::error_code error_read_file = read_file(fd.value(), buffer);
    // Error en el read file.
    //
    if (error_read_file)
    {
      close(fd.value());
      error_read_file.message();
      std::error_code error(errno, std::system_category());
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
      std::error_code error(errno, std::system_category());
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

std::error_code write_file(int fd, std::vector<uint8_t>& buffer)
{
  ssize_t bytes_write = write(fd, buffer.data(), buffer.size());
  if(bytes_write < 0)
  {
    return std::error_code(errno, std::system_category());
  }
  return std::error_code(0, std::system_category());
}

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
    std::cerr << "Error make ip address.\n";
    std::error_code error(errno, std::system_category());
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
    std::cerr << "Error at make socket.\n";
    std::error_code error(errno, std::system_category());
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
    return std::error_code(errno, std::system_category());
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

std::string getenv(const std::string& name) 
{
  char* value = getenv(name.c_str());
  if (value) {
    return std::string(value);
  }
  else {
    return std::string();
  }
}

void Usage() 
{
  std::cout << "Modos de uso: ./netcp [-h] ORIGEN\n"
            << "\t      ./netcp ARCHIVO\n"
            << "\t      ./netcp [-1] [-2] -c COMANDO [ARG...]\n"
            << "\t      ./netcp -l ARCHIVO\n"
            << "\t      ./netcp -l -c COMANDO [ARG...]\n";
}

std::optional<program_options> parse_args(int argc, char* argv[])
{
  if (argc < 2) return std::nullopt;
  std::vector<std::string_view> args(argv + 1, argv + argc);
  program_options options;
  for (auto it = args.begin(), end = args.end(); it != end; ++it)
  {
    if (*it == "-h" && options.read_comm == false)
    {
      options.show_help = true;
    }
    else if (*it == "-l" && options.read_comm == false)
    {
      options.listening_mode = true;
    }
    else if (*it == "-1" && options.read_comm == false)
    {
      options.normal_out = true;
    }
    else if (*it == "-2" && options.read_comm == false)
    {
      options.error_out = true;
    }
    else if (*it == "-c" && options.read_comm == false)
    {
      options.command_mode = true;
    }
    else if ( options.command_mode == true /*&& options.listening_mode == false*/)
    {
      options.read_comm = true;
      options.args.emplace_back(*it);
    }
    else if (options.listening_mode == true && options.command_mode == false)
    {
      options.output_filename = *it;
    }
    else
    {
      options.input_file = *it;
    }
  }
  options.read_comm = false;
  return options;
}

void term_signal_handler(int signum)
{
  const char* message;

  if (signum == SIGTERM)
  {
    message = "Señal SIGTERM recibida.\n";
  }
  else if (signum == SIGINT)
  {
    message = "Señal SIGINT recibida.\n";
  }
  else
  {
    message = "Señal SIGTERM recibida.\n";
  }
  
  write(STDOUT_FILENO, message, strlen(message));
}

