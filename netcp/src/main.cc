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

// Imprimir el mensaje y la direccion remitente
/*
std::cout << std::format("El sistema '{}' envió el mensaje '{}'\n",
    ip_address_to_string(remote_address),
    message.text.c_str()
)
*/

/*
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
  if (bytes_read < 0)
  {
    return std::error_code(errno, std::system_category());
  }
  buffer.resize(bytes_read);
  return std::error_code(0, std::system_category());
*/
#include "netcp.h"

std::atomic<bool> quit_requested = false;
std::string prog_name;

int main(int argc, char **argv) {
  prog_name = argv[0];

  struct sigaction term_action = {0};
  term_action.sa_handler = &term_signal_handler;
  term_action.sa_flags = 0;

  sigaction(SIGTERM, &term_action, NULL);
  sigaction(SIGINT, &term_action, NULL);
  sigaction(SIGHUP, &term_action, NULL);
  

  auto options = parse_args(argc, argv);
  if (!options)
  {
    std::cerr << "Error!" << std::endl;
    return EXIT_FAILURE;
  }
  //Controlador de las opciones que tiene el programa
  if (options.value().show_help) 
  {
    Usage();  
  }
  else if (options.value().command_mode)
  {
    std::cout << "Comando activado.\n";
    subprocess::stdio redirected_io;
    if (options.value().listening_mode)
    {
      std::cout << "Escucha.\n";
      for(auto& p : options.value().args)
      {
        std::cout << p << " ";
      }
    std::cout << "\n";
    redirected_io = subprocess::stdio::in;
    
    subprocess comm_netcp(options.value().args, redirected_io);
    }
    else 
    {
      std::cout << "Envio.\n";
      for(auto& p : options.value().args)
      {
        std::cout << p << " ";
      }
      std::cout << "\n";
      if (options.value().normal_out && !options.value().error_out)
      {
        redirected_io = subprocess::stdio::out;
      }
      else if (options.value().error_out && !options.value().normal_out)
      {
        redirected_io = subprocess::stdio::err;
      } 
      else if (options.value().normal_out && options.value().error_out)
      {
        redirected_io = subprocess::stdio::outerr;
      }
      else
      {
        redirected_io = subprocess::stdio::out;
      }

      subprocess comm_netcp(options.value().args, redirected_io);

      std::error_code error_exec = comm_netcp.exec();
      if (error_exec)
      {
        //errores errorrosos de la erroreria
        std::cout << "Error " << error_exec.value() << ": " << error_exec.message() << std::endl;
        return error_exec.value();
      }
    }
  }
  else
  {
    if (options.value().listening_mode)
    {
      std::error_code error_recieve = netcp_recieve_file(options.value().output_filename);
      if (error_recieve)
      {
        std::cout << "Error " << error_recieve.value() << ": " << error_recieve.message() << std::endl;
        return error_recieve.value();
      }
    }
    else
    {
      std::error_code error_send = netcp_send_file(options.value().input_file);
      if (error_send)
      {
        std::cout << prog_name << ": Error " << error_send.value() << ": " << error_send.message() << std::endl;
        return error_send.value();
      }
    }
  }
  return EXIT_SUCCESS;
}


/*
(1) pipe
  
  int fdp[2];
  pipe(fdp);  --> [ 0 ] [ 1 ]    0 -> RD   1 -> WR

(2) fork
la funcion fork devuelve el pid del hijo

(3) Padre
-> modo de envio
    llamar al read_file(
                fdp[0] modo lectura
                );
    close(fdp[1]);
->modo de eschuca
    llamar al write_file(
                fdp[1] modo escritura
                );
    close(fdp[0]);
    
    Hijo
-> modo de envio
    fdp[1]     [    ]
      1        [    ]

    dup2(fdp[1], STDOUT_FILENO)
    close(fdp[0]);

-> modo de escucha

    fdp[0]     [    ]
      0        [    ]
  
  dup2(fdp[0], STDIN_FILENO);
  close(fdp[1]);

(4) HIJO -> exec


(5) 
    int status;
    while(flag)
    {
      int res = wait(pid, &status
          W_NOHANG);
      if(res != 0)
      {
        flag = flase;
      }
    }
    kill(pid, SIGTERM);






*/