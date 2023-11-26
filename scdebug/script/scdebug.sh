#!/bin/bash
#falta la parte 4.3

PROGNAME=$(basename $0)

opK=
opSto=
opcNattch=
opcPattach=
listaProg=
opcV=
opcVall=
ayuda=
lista=
leelista=0
leeProg=0

error_exit()
{
    echo "${PROGNAME} : ${1:-"Error desconocido"}" 1>&2
    exit 1
}

usage() 
{ 
    echo "scdebug.sh [-h] [-sto arg] [-v | -vall] [-nattch progtoattach] [prog [arg1 ...]]"
    echo "scdebug [-h] [-sto arg] [-v | -vall] [-k] [prog [arg …] ] [-nattch progtoattach …] [-pattch pid1 … ]"
}

buscarPID()
{
    PID=$(ps x --sort=etime -o comm,pid | grep "$commNattch" | tr -s " " | cut -d ' ' -f 2 | head -1)
}



trace()
{
    uuid=$(uuidgen)
    directorio="${HOME}/.scdebug/$(prog ${listaProg})"
    mkdir -p "${directorio}" || error_exit "No pudo crear un nuevo directorio"
    comandoTrace="strace ${argsSto} -o ${directorio}/trace_${uuid}.txt ${listaProg}"
    ${comandoTrace} & 
}

Nattch()
{
    while [ $# -gt 0 ]; do
        commNattch="$1"
        buscarPID
        uuid=$(uuidgen)
        directorio="${HOME}/.scdebug/${commNattch}"
        mkdir -p "${directorio}" || error_exit "No pudo crear un nuevo directorio"
        comandoNattch="strace ${argsSto} -p ${PID} -o ${directorio}/trace_${uuid}.txt"
        ${comandoNattch} &
        shift
    done
}

opcionPattch()
{
    while [ $# -gt 0 ]; do
        pidPattch="$1"
        uuid=$(uuidgen)
        commPattch=$(ps -p ${pidPattch} -o cmd --no-header)
        directorio="${HOME}/.scdebug/$(prog $commPattch)"
        mkdir -p "${directorio}" || error_exit "No pudo crear un nuevo directorio"
        comandoPattch="strace ${argsSto} -p ${pidPattch} -o ${directorio}/trace_${uuid}.txt"
        ${comandoPattch} &
        shift
    done
}

opcionV() 
{
    while [ $# -gt 0 ]; do
        progV="$1"
        if [ -n "$opcPattach" ]; then
            echo "c"
            commPid=$(ps -p ${progV} -o comm --no-header)
            progV="$commPid"
        fi
        directorio=${HOME}/.scdebug/${progV}
        traceNameFile=$(ls ${directorio} --sort time | head -1)
        timeFile=$(ls -l --time-style=+'%Y-%m-%d %H:%M:%S' ${directorio} | grep "${traceNameFile}" | cut -d ' ' -f6-7)

   cat << _EOF_
    ============================================================================================================
    =============== COMMAND: ${progV} =======================
    =============== TRACE FILE: ${traceNameFile} =================
    =============== TIME: ${timeFile} ==============
    ============================================================================================================
_EOF_
    shift
    done
}

opcionVall()
{
    while [ $# -gt 0 ]; do
        progVall="$1"
        directorio=${HOME}/.scdebug/${progVall}
        listaTrace=$(ls $directorio)
        for trace in $listaTrace; do
            timeFile=$(ls -l --time-style=+'%Y-%m-%d %H:%M:%S' ${directorio} | grep "${trace}" | cut -d ' ' -f6-7)
    cat << _EOF_
    ============================================================================================================
    =============== COMMAND: ${progVall} =======================
    =============== TRACE FILE: ${trace} =================
    =============== TIME: ${timeFile} ==============
    ============================================================================================================
_EOF_
done
shift
done
}

opcionK()
{
    listapid=$(ps -x --no-header --sort=etime | tr -s " " | cut -d ' ' -f 2)

    for pid in $listapid; do
    if  [ -f /proc/$pid/status ]; then
    tracerpid=$(cat /proc/$pid/status | grep TracerPid | tr -s " " | cut -d ':' -f 2)
        if [ "$tracerpid" -ne 0 ]; then
            kill -9 $pid
        fi
    fi
    done

}

procesosUSER()
{
    listapid=$(ps -x --no-header --sort=etime | tr -s " " | cut -d ' ' -f 2)

    for pid in $listapid; do
    if  [ -f /proc/$pid/status ]; then
        tracerpid=$(cat /proc/$pid/status | grep TracerPid | tr -s " " | cut -d ':' -f 2)
        if [ "$tracerpid" -ne 0 ]; then
            procesospid=$(ps -u $USER --no-header --sort=etime | grep $pid | tr -s " " | cut -d ' ' -f 5)
            nombretraceador=$(ps -u $USER --no-header --sort=etime | grep $tracerpid |  tr -s " " | cut -d ' ' -f 5)
            echo $pid $procesospid $tracerpid $nombretraceador
        fi
    fi
    done
}

opciones()
{
while [ -n "$1" ]; do

    case "$1" in

        -h | --help )
            ayuda=1;
            ;;
        -k )
        opk=1;
            ;;
        -v )
            opcV=1;
            leelista=1;
	    if [ "$leeProg" -eq 1 ]; then
		    leeProg=2
	    fi
            ;;
        -vall )
            opcVall=1;
            leelista=1;
	    if [ "$leeProg" -eq 1 ]; then
		    leeProg=2
	    fi
            ;;
        -sto )
            opcSto=1;
            leelista=0;
	    if [ "$leeProg" -eq 1 ]; then
		leeProg=2
	    fi
            shift
            if [ -z "$1" ]; then
                error_exit
            fi
            argsSto="$1"
            ;;
        -nattch )
        opcNattch=1;
        leelista=1;
	    if [ "$leeProg" -eq 1 ]; then
		    leeProg=2
	    fi
            ;;
        -pattch )
        opcPattach=1;
        leelista=1;
	    if [ "$leeProg" -eq 1 ]; then
		    leeProg=2
	    fi
            ;;
        * ) 
        if [ "$leelista" -ne 1 -a "$leeProg" -ne 2 ]; then
		leeProg=1
		listaProg+="$1 "
        elif [ "$leelista" -eq 1 ]; then
            lista+="$1 "
        else
            error_exit
        fi
	    ;;
     esac
     shift
done
}

prog()
{
    echo "$@" | tr -s " " | cut -d ' ' -f 1
}

args()
{
    echo "$@" | tr -s " " | cut -d ' ' -f2-
}

check()
{
    if [ -z "$ayuda" -a -z "$opk" ]; then
        if [ -z "$lista" -a -z "$listaProg" ]; then
            error_exit "Faltan parametros."
        fi
    fi

    if [ -n "$opcV" -a -n "$opcVall" ]; then
        error_exit "Opciones incompatibles. No se puede usar -v y -vall al mismo tiempo."
    fi

}

main()
{
    opciones "$@"
    check
    
    if [ -n "$ayuda" ]; then
        usage
    fi

    if [ -z "$opcV" -a -z "$opcVall" ]; then
        procesosUSER
    fi

    if [ -n "$opk" ]; then
        opcionK
    fi

    if [ -n "$opcNattch" ]; then
        Nattch $lista
    fi

    if [ -n "$opcPattach" ]; then
        opcionPattch $lista
    fi 

    if [ -n "$listaProg" ]; then
        trace
    fi

    if [ -n "$opcV" ]; then
        opcionV $lista
    fi

    if [ -n "$opcVall" ]; then
        opcionVall $lista
    fi

}

if [ $# -le 0 ]; then
    error_exit "Parámetros incorrectos, Ejecute ./scdebug.sh --help"
fi
main "$@"