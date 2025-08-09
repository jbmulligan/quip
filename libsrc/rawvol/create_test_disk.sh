#!/bin/bash

make_one(){
	if [[ -e $1 ]]; then
		echo "File $1 already exists."
	else
		sudo mkfile -v 1g $1
	fi
}

make_one test_disk1
make_one test_disk2

