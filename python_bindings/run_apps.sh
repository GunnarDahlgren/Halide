#!/bin/bash
# Usage: run_apps.sh /path/to/build/directory

PYTHON=python3

# Operate in the build directory, so that output files don't pollute the top-level directory.
cd $1

export PYTHONPATH="$1:$PYTHONPATH"
echo "PYTHONPATH ==" $PYTHONPATH

FAILED=0

# separator
S=" --------- "
Sa=" >>>>>>>> "
Sb=" <<<<<<<< "

if [ -z "$ROOT_DIR" ]; then ROOT_DIR=$(dirname $0); fi

for i in ${ROOT_DIR}/apps/*.py
do
  echo $S $PYTHON $i $S
  $PYTHON $i
  if [[ "$?" -ne "0" ]]; then
    echo "$Sa App failed $Sb"
	let FAILED=1
	break
  fi
done

if [[ "$FAILED" -ne "0" ]]; then
  exit -1
else
  echo "$S (all applications ran) $S"
fi
