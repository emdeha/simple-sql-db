#!/bin/bash

valgrind --leak-check=yes --suppressions="$PWD/.valgrind.supp" "$PWD/main"
