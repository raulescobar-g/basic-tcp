#!/bin/bash

gcc server.c -fsanitize=address -o bin/server
./bin/server "$@" 
