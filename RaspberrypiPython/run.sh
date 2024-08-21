#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

source ${SCRIPT_DIR}/.venv/bin/activate
while :
do
   python ${SCRIPT_DIR}/robotcontroller.py
   sleep 2
done