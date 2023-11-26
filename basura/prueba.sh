#!/bin/bash

echo -n "Introduce unnúmero entre 1 y 3, ambos incluidos > "
read character
case $character in
	1 ) echo "Has introducido un uno."
			;;
	2 ) echo "Has introducido un dos."
			;;
	3 ) echo "Has introducido un tres."
			;;
	paco ) echo "Para que pones paco cabron."
			;;
	* ) echo "No has introducido un número dentro del rango."
esac

