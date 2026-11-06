#!/bin/sh

cd "$(dirname "$0")/Core-src" || exit 1
case "$1" in
	clean) 
		echo "Cleaning up"
		make clean
		echo "Build has been cleaned up"
		exit 1
		;;
	*)
		echo "Starting Build of BeAE"		
		make
		echo "The build has finished"
		echo "BeAE/BeAE to run"
		;;
esac
