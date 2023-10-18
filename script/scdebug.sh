#!/bin/bash

##CONSTANTES
PROGNAME=$(basename $0)
##Variables
prog=
args=
sto_option=
nattch_option=
pid=
ntrace_option=
prefix=
set -e

Ntrace() {

  dir="$HOME/.scdebug/"

  for arg in "$dir"*; do

  total_trace=$(ls $arg | wc -l)
  process=$(basename $arg)
  echo "$process"
  echo "$total_trace"
  done


}

error_exit() {
    echo "${PROGNAME}: ${1:-"Error desconocido"}" 1>&2
    exit 1
}

#Funcion que busca el PID sobre el prog mas reciente
SearchMostRecentPID() {
  ./ptbash
  pid=$(ps -u $USER --no-header --sort=etime | grep "$nattch_option" | tr -s " " | cut -d ' ' -f 2)
}

#Funcion para comprobar que los parametros sean correctos
CheckParameters() {
  if [ $# -le 0 ]; then
    echo "Parametros incorrectos"
    exit 1
    fi 
}

#Funcion para mostrar la ayuda sobre el programa
Usage() {
  echo "scdebug [-h] [-sto arg] [-v | -vall] [-nattch progtoattach] [prog [arg1 ...]]"
}

ChooseOption() {
  #Mientras el total de argumentos sea > 0
  while [ $# -gt 0 ]; do
  case "$1" in
    #Opcion -h / --help
    -h|--help)
      Usage  #Mostramos ayuda por pantalla
      exit 0
      ;;
      #Opcion -sto
    -sto)
      shift  
      sto_option="$1"
      shift
      ;;
      #Opcion -v / -vall (no hacen nada por ahora)
    -v | -vall)
      ;;
      #opcion -nattch
    -nattch)
      shift  #Avanzamos $1
      nattch_option="$1"  #Guardamos el file
      SearchMostRecentPID  #Buscamos el PID mas reciente con $1
      ;;
      #opcion -Ntrace
    Ntrace)
      ntrace_option="$1"
      shift
      ;;
    --prefix )
      shift
      if [ -n "$1" ]; then
        prefix="$1"
      else
        error_exit "falta directorio"
      fi
      ;;
    *)
      #Nombre del programa a evaluar
      prog="$1"  #Guardamos el nombre del programa y hacemos shift para cambiar $1
      shift
      args="$@"  #Guardamos el resto dNtrace
      break
      ;;
    esac
done
}

PROG() {

  #Creamos el uuid especifico
  uuid=$(uuidgen)
  #Creamos una variable con la direccion del directorio
  if [ -z "$prefix" ]; then
    dir="$HOME/.scdebug/$prog"
  else
    dir="$prefix"
  fi

  #Creamos el directorio
  mkdir -p "$dir"
  
  # Si la opcion -nattch esta vacia
  if [ -z "$nattch_option" ]; then
  #Ejecutamos el comando sin las opciones de nattch
  command1="strace $sto_option -o $dir/trace_$uuid.txt $prog $args"
  ./l2t $command1  > /dev/null &
  else
  #Ejecutamos con las opciones de nattch
  command2="strace $sto_option -p $pid -o$dir/trace_$uuid.txt"
  ./l2t $command2  > /dev/null &
  fi
}

#Funcion principal 
MainProgram() {
  #Comprobamos que se hayan pasado los parametros correctos
  CheckParameters "$@"
  if [ "$?" -eq 0 ]; then
    if [ "$1" = "Ntrace" ]; then
      Ntrace
    else
      #Calculamos el conjunto de opciones que se usaran en el comando
      ChooseOption "$@"
      #Ejecutamos el comando con nuestras opciones
      PROG 
    fi
  fi
}


MainProgram "$@"