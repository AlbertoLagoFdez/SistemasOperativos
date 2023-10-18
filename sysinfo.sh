#!/bin/bash

#sysinfo - Un script que informa del estado del sistema


#### Constantes

 PROGNAME=$(basename $0)


TITLE="Información de mi sistema para $HOSTNAME"
RIGTH_NOW=$(date +"%x %r%Z")
TIME_STAMP="Actualizada el $RIGTH_NOW por $USER"

#### Estilos

TEXT_BOLD=$(tput bold)
TEXT_GREEN=$(tput setaf 2)
TEXT_RESET=$(tput sgr0)
TEXT_ULINE=$(tput sgr 0 1)

#### Funciones

error_exit()
{

#   ---------------------------------------------------------
#   Función para salir en caso de error fatal
#   			Acepta 1 argumento:
#									cadena conteniendo un mensaje descriptivo del error
#		---------------------------------------------------------

	echo "${PROGNAME}: ${1:-"Error desconocido"}" 1>&2 # cerr stderr
	exit 1
}

system_info()
{
	echo "${TEXT_ULINE}Versión del sistema${TEXT_RESET}"
	echo
	uname -a
	
}

show_uptime()
{
	echo "${TEXT_ULINE}Tiempo de encendido del sistema${TEXT_RESET}"
	echo
	type -P uptime || error_exit "El comando uptime no no está instalado."
	
}

drive_space()
{
	echo "${TEXT_ULINE}Espacio en disco${TEXT_RESET}"
	echo
	df
	
}

home_space()
{
	echo "${TEXT_ULINE}Espacio en home por usuario${TEXT_RESET}"
	if [ "$USER" = "root" ]; then
		echo "${TEXT_ULINE}Bytes${TEXT_RESET}   ${TEXT_ULINE}Directorio${TEXT_RESET}"
		du -s /home/* | sort -nr
	else
		echo "${TEXT_ULINE}Bytes${TEXT_RESET}   ${TEXT_ULINE}Directorio${TEXT_RESET}"
		du -s /home/*$USER | sort -nr
	fi
}

#### Programa principal

usage()
{
	echo "usage: sysinfo [ -f file ] [ -i ] [ -h ]"
}

write_page()
{
	# El script-here se puede indentar dentro de la funcion si
	# se san tabuladores y "<<-EOF" en lugar de "<<"
	cat << -EOF
$TEXT_BOLD$TITLE$TEXT_RESET

$(system_info)

$(show_uptime)

$(drive_space)

$(home_space)

$TEXT_GREEN$TIME_STAMP$TEXT_RESET
-EOF
}

#Opciones por defecto. ojo. lo normal es ponerlas al principio de script, junto a las
# constantes, para qe sean fáciles de encontrar y modificar por otros programadores
interactive=
filename=sysinfo.txt

#procesar la linea de comandos del script para leer las opciones
while [ "$1" != "" ]; do
	case $1 in
		-f | --file )
			shift
			filename=$1
			;;
		-i | --interactive )
			interactive=1
			;;
		-h | --help )
			usage
			exit
			;;
		* )
			usage
			error_exit "Opción desconocida"
	esac
	shift
done

if [ "$interactive" == 1 ]; then
	echo -n "¿En que archivo quieres guardar la salida?: "
	read filename
fi

	# Generar el informe del sistema y guardarle en el archivo indicado
	# en $filename
	write_page > $filename
