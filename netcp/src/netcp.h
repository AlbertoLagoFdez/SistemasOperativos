#ifndef NETCP_H
#define NETCP_H

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <expected>
#include <format>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <atomic>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <system_error>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <experimental/scope>

using open_file_result = std::expected<int, std::error_code>;
using make_socket_result = std::expected<int, std::error_code>;
using std::experimental::scope_exit;

struct program_options
{
  std::string output_filename;
  std::string input_file;     //Este seria el mensaje
  bool show_help = false;
  bool listening_mode =  false;
  bool command_mode = false;
  bool error_out = false;
  bool normal_out = false;
  bool read_comm = false;
  std::vector<std::string> args;
};

class subprocess
{
 public:
  
  enum class stdio
  {
    in,
    out,
    err,
    outerr
  };

  subprocess(
      const std::vector<std::string>& args,
      subprocess::stdio redirected_io
  );

  bool is_alive();

  std::error_code exec();
  std::error_code wait();
  std::error_code kill();

  pid_t pid();

  int stdin_fd();
  int stdout_fd();

 private:
  //atributos
  std::vector<std::string> args_;
  subprocess::stdio redirected_io_;

};

[[nodiscard]]
open_file_result open_file(const std::string& path, int flags, mode_t mode);

[[nodiscard]]
std::error_code read_file(int fd, std::vector<uint8_t>& buffer);

std::optional<sockaddr_in> make_ip_address(
    const std::optional<std::string> ip_address, uint16_t port);

make_socket_result make_socket(
    std::optional<sockaddr_in> address = std::nullopt);

std::error_code send_to(int fd, const std::vector<uint8_t>& message,
    const sockaddr_in& address);

std::error_code netcp_send_file(const std::string& filename);

std::error_code write_file(int fd, std::vector<uint8_t>& buffer);

std::error_code recieve_from(int fd, std::vector<uint8_t>& buffer,
    sockaddr_in& address);

std::error_code netcp_recieve_file(const std::string& filename); 

std::string getenv(const std::string& name);

void Usage();

std::optional<program_options> parse_args(int argc, char* argv[]);

void term_signal_handler(int signum);

#endif