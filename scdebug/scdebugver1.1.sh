#!/bin/bash

# ===========================================================================================================

optionSto=""
optionNattch=""

# ===========================================================================================================

Usage()
{
    echo "scdebug [-h] [-sto arg] [-v | -vall] [-k] [prog [arg …] ] [-nattch progtoattach …] [-pattch pid1 … ]"
}

searchRecentPID() 
{
    #./ptbash
    PID=$(ps x -o comm,pid | grep "$optionNattch" | tr -s " " | cut -d ' ' -f 2 | head -1 )
}

trace()
{
    uuid=$(uuidgen)
    directorio="${HOME}/.scdebug/${programa}"
    mkdir -p "${directorio}"

    comando1="strace ${optionSto} -o ${directorio}/trace_${uuid}.txt ${programa} ${argumentos}"
    $comando1 & > ${directorio}/trace_${uuid}.txt &
}

#version de nattch que permite una lista de programas
Nattch()
{
    while [ $# -gt 0 ]; do
	    optionNattch="$1"
  	  searchRecentPID
    	uuid=$(uuidgen)
    	directorio="${HOME}/.scdebug/${optionNattch}"
    	mkdir -p "${directorio}"
    	comando2="strace ${optionSto} -p ${PID} -o ${directorio}/trace_${uuid}.txt"
    	${comando2} & 
    	shift
    done
}

Pattch()
{
    listaProcesos=$(ps -u ${USER} --no-header --sort=etime | tr -s " " | cut -d ' ' -f 2)
    for pids_pattch in $listaProcesos; do
        strace -p $pids_pattch &
    done
}

optionV()
{
    directorio=${HOME}/.scdebug/${varV}
    traceNameFile=$(ls ${directorio} --sort time | head -1)
    timeFile=$(ls -l ${directorio} | grep "${traceNameFile}" | cut -d ' ' -f6-9)

    cat << _EOF_

    ============================================================================================================
    =============== COMMAND: ${varV} =======================
    =============== TRACE FILE: ${traceNameFile} =================
    =============== TIME: ${timeFile} ==============
    ============================================================================================================

_EOF_
}

optionVall() 
{
    directorio=${HOME}/.scdebug//${varVALL}
    listaTrace=$(ls $directorio)
    for trace in $listaTrace; do
    timeFile=$(ls -l ${directorio} | grep "${trace}" | cut -d ' ' -f6-9)
    cat << _EOF_
    ============================================================================================================
    =============== COMMAND: ${varVALL} =======================
    =============== TRACE FILE: ${trace} =================
    =============== TIME: ${timeFile} ==============
    ============================================================================================================
_EOF_
done
}

procesosUSER()
{
    listapid=$(ps -u $USER --no-header --sort=etime | tr -s " " | cut -d ' ' -f 2)

    for pid in $listapid; do
    tracerpid=$(cat /proc/$pid/status | grep TracerPid | tr -s " " | cut -d ':' -f 2)
    procesospid=$(ps -u $USER --no-header --sort=etime | grep $pid | tr -s " " | cut -d ' ' -f 5)
    echo $pid $procesospid $tracerpid
    done
}

optionK()
{
    procesosUSER=$(ps -u $USER -o pid --sort=start_time --no-header | tr -s " " | cut -d ' ' -f 2)

    for pid in $procesosUSER; do
        if [ "$pid" -gt 1 ]; then
            if [ -f /proc/${pid}/status ]; then
                tracerPID=$(cat /proc/${pid}/status | grep "TracerPid" | tr -d " " | cut -d ':' -f 2)
                if [[ "$tracerPID" == 1 ]]; then
                    kill -9 $pid
                    kill -9 $tracerPID
                fi
            fi
        fi
    done
}

options()
{
    while [ -n "$1" ]; do
    case "$1" in
        -h | --help )
            Usage
            exit 0
            ;;
        -k )
            shift
            optionK
            ;; 
        -sto )
            shift
            optionSto="$1"
            shift
            ;;
        -v )
            shift
            varV="$1"
            shift
            ;;
        -vall )
            shift
            varVALL="$1"
            shift
            ;;
        -nattch )
            shift
            Nattch "$@"
            shift
            ;;
        -pattch )
            shift
            Pattch
            shift
            ;;
        * )
            programa="$1"
            shift
            argumentos="$@"
            trace
            break
            ;;
    esac
    done
}

main()
{
    if [ $# -le 0 ]; then
    echo "Parametros incorrectos"
    exit 1
    fi
    if [ $1 = "-v" ]; then
        shift
        varV="$1"
        optionV
    elif [ $1 = "-vall" ]; then
        shift
        varVALL="$1"
        optionVall
    else
#procesosUSER
        options "$@"
    fi
    #procesosUSER
}

main "$@"
