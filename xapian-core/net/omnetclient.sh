#!/bin/bash

echo "NETCLIENT> fd $1"

echo "BOO!" >&$1

read stuff <&$1

echo "NETCLIENT> got $stuff"

