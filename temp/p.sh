#!/bin/bash
#Mostrar los nombres de directorios en el directorio de trabajo actual ordenados por tiempo de modificación

directorios() {
	ls -l --time-style=+%s -t | tr -s ' ' | tail -n+2 | grep '^d' | cut -d ' ' -f7-
}

DIRS=$(directorios)
cat << -EOF
Los directorios son:
--------------------
$DIRS
--------------------
-EOF
