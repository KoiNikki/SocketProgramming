#!/bin/bash  

python3 udp/python/server.py &
 
sleep 1  

python3 udp/python/client.py  

sleep 5  
kill %1