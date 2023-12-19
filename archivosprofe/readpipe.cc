#include <iostream>
#include <vector>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <unistd.h>

int main(int argc, char **argv)
{
  // Argumentos
  if(argc<3)
  {
    std::cerr << "Wrong number of arguments\n";
    return EXIT_FAILURE;
  }
  std::string fileName(argv[1]);
  std::vector<std::string> command_args;
  for(int i = 2 ; i < argc ; i++) {
    command_args.emplace_back(argv[i]);
  }
  // Archivo
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
  if(fd<0) 
  {
    std::cerr << "Error at OPEN\n";
    return EXIT_FAILURE;
  }
  // Pipe
  int fds[2]; // fds[0] es el extremo de lectura y fds[1] el extremo de escritura
  int pipe_result = pipe(fds);
  if (pipe_result < 0) {
    std::cerr << "Error AT PIPE\n";
    return EXIT_FAILURE;
  }

  // Proceso HIJO
  pid_t pid = fork();
  if(pid > 0) 
  {
    // Proceso Padre
    // Utilizo el extremo de lectura
    close(fds[1]);
    char buffer[256];
    int nbytes{0};
    do{
      nbytes=read(fds[0], buffer, 256);
      write(fd, buffer, nbytes);
    }while(nbytes>0);

    int status;
    bool flag = true;
    while (flag)
    {
      int result = waitpid(pid, &status, WNOHANG);
      if(result!=0)
      {
        flag = false;
      }
    }
    
    close(fd);
    return EXIT_SUCCESS;
  }
  
  else if(pid == 0) 
  {
    // Proceso Hijo
    
    // EL proceso hijo escribe en el extremo de escritura en fds[1]
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    close(fds[1]);
    std::vector<const char*> exec_args;
    for(auto &str : command_args) 
    {
      exec_args.push_back(str.c_str());
    }
    exec_args.push_back(nullptr);
    int exec_result = execvp(exec_args[0], const_cast<char *const *>(exec_args.data()));
    //ERROR de execvp
    // no tiene if porque si pasa a la siguiente linea es porque execvp a fallado.
    close(fd);
    std::cerr << "Error executing command\n";
    return errno;



  }
  else 
  {
    // ERROR
    std::cerr << "Error en FORK\n";
    close(fd);
    return EXIT_FAILURE;
 }

  close(fd);
  return EXIT_SUCCESS;
}