#!/bin/bash

if [[ ! -e ../docs ]]; then
	echo "Creatings docs directory..."
	mkdir ../docs
fi
if [[ ! -e ../docs/quip_web_manual ]]; then
	echo "Creating quip_web_manual directory..."
	mkdir ../docs/quip_web_manual
fi

if [[ -e quip ]]; then
	cp -R quip/*.html ../docs/quip_web_manual
elif [[ -e quip_html ]]; then
	cp -R quip_html/*.html ../docs/quip_web_manual
else
	echo "ERROR: did not find subdirectory quip or quip_html!?"
	exit 1
fi

