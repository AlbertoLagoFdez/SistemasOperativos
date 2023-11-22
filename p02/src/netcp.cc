#include <iostream>
#include <vector>
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


std::optional<sockaddr_in> make_ip_address(
    const std::optional<std::string> ip_address, uint16_t port=0);

make_socket_result make_socket(
    std::optional<sockaddr_in> address = std::nullopt);


std::error_code send_to(int fd, const std::vector<uint8_t>& message,
     const sockaddr_in& address)
{

}

struct program_options
{
bool show_help = false;
std::string output_filename;
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
  std::cout << "./netcp [-h] [-o] [filename] ";
}



std::optional<program_options> parse_args(int argc, char* argv[])
{
  std::vector<std::string_view> args(argv + 1, argv + argc);
  program_options options;
  for (auto it = args.begin(), end = args.end(); it != end; ++it)
  {
    if (*it == "-h" || *it == "--help")
    {
      options.show_help = true;
    }
    if (*it == "-o" || *it == "--output")
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
// Opciones adicionales...
  }
return options;
}



int main(int argc, char **argv) {
  auto options = parse_args(argc, argv);
  if (!options)
  {
    return EXIT_FAILURE;
  }
// Usar options.value() para acceder a las opciones...
  if (options.value().show_help) 
  {
    Usage();  
  }

  int flags = O_RDONLY | O_CREAT;
  mode_t mode = 0666;
  open_file_result result = open_file(options.value().output_filename, flags, mode);
  
  if (!result) {
    //return result.error();
    std::cerr << "¡Error! No se puede abrir el archivo." << std::endl;
    return EXIT_FAILURE;
  }
  int fd = *result;

  std::vector<uint8_t> buffer(1024);
  std::error_code error = read_file(fd, buffer);
  if (error)
  {
    std::cerr << std::format("Error ({}): {}\n", error.value(),
    error.message());
  }

  auto address = make_ip_address("127.0.0.1", 8080);
  auto result = make_socket(address.value());
  if (result)
  {
    sock_fd = *result;
  }

  return EXIT_SUCCESS;
}