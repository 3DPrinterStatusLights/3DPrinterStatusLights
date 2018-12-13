#!/bin/bash

# This bash script will run the program every 5 seconds indefinitely.

while :
do
  sudo /root/ECE-4180-Project/push $1
  sleep 5;
done
