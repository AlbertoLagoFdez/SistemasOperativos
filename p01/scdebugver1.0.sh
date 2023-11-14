#!/bin/bash

# =========  CONSTANTES  =========

PROGNAME=$(basename $0)

# =========  VARIABLES   =========

uuid=
optionSto=
optionNattch=

# =========  FUNCIONES   =========

Usage()
{
    echo "scdebug [-h] [-sto arg] [-v | -vall] [-nattch progtoattach] [prog [arg1 ...]]"
}

searchRecentPID() 
{
    #./ptbash
    PID=$(ps -u $USER --no-header --sort=etime | grep "$optionNattch" | tr -s " " | cut -d ' ' -f 2 | head -1 )
}

optionV()
{
    directorio=${HOME}/.scdebug/${programaV}
    traceNameFile=$(ls ${directorio} --sort time | head -1)
    timeFile=$(ls -l ${directorio} | grep "${traceNameFile}" | cut -d ' ' -f6-9)

    cat << _EOF_

    ==============================================================================
    =============== COMMAND: ${programaV} =======================
    =============== TRACE FILE: ${traceNameFile} =================
    =============== TIME: ${timeFile} ==============
    ==============================================================================

_EOF_
}

optionVall()
{
    directorio=${HOME}/.scdebug//${programaVall}
    listaTrace=$(ls $directorio)
    for trace in $listaTrace; do
    timeFile=$(ls -l ${directorio} | grep "${trace}" | cut -d ' ' -f6-9)
    cat << _EOF_
    ==============================================================================
    =============== COMMAND: ${programaVall} =======================
    =============== TRACE FILE: ${trace} =================
    =============== TIME: ${timeFile} ==============
    ============================================================================================
_EOF_
done
}

Opciones()
{
    while [ $# -gt 0 ]; do
    case "$1" in
        --count | -coutn-name)
            ls ${HOME}/.scdebug/ | wc -l
            exit 0
            ;;
        -h | --help )
            Usage
            exit 0
            ;;
        -sto )
            shift
            optionSto="$1"
            shift
            ;;
        -v )
            shift
            programaV="$1"
            shift
            optionV
            exit 0
            ;;
        -Vall )
            shift
            programaVall="$1"
            shift
            optionVall
            exit 0
            ;;
        -nattch )
            shift
            optionNattch="$1"
            shift
            ;;
        *)
        programa="$1"
        shift
        args="$@"
        break
        ;;
    esac
    done
}

ProgramaPrincipal()
{
    uuid=$(uuidgen)
    directorio="${HOME}/.scdebug/${programa}"
    mkdir -p "$directorio"

    if [ -z "$optionNattch" ]; then
    comando1="strace ${optionSto} -o ${directorio}/trace_${uuid}.txt ${programa} ${args}"
    $comando1 2>1 ${directorio}/trace_${uuid}.txt &
    else
    echo "hola"
    directorioNattch="${HOME}/.scdebug/${optionNattch}"
    searchRecentPID
    comando2="strace ${optionSto} -p ${PID} -o ${directorioNattch}/trace_${uuid}.txt"
    echo "${comando2}"
    $comando2 2>1 ${directorioNattch}/trace_${uuid}.txt &
    fi
}

Main()
{
    if [ $# -le 0 ]; then
    echo "Parametros incorrectos"
    exit 1
    fi
    Opciones "$@" 
    ProgramaPrincipal
}

Main "$@"