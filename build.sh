#!/bin/bash

set -euo pipefail

g++ -std=c++20 -Wall -Wextra -Wpedantic main.cpp -o compiler
