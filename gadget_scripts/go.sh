#!/bin/bash
#Array of Storage Paths
declare -A paths
# go command
go() {
  local parameter=$1
  local path=${paths[$parameter]}

  if [[ -n $path ]]; then
    cd "$path" || echo "Unable to jump to path: $path"
  else
    echo "Parameter not found: $parameter"
  fi
}
# adir command
adir() {
  local parameter=$1
  local path=$2
 if [[ -z $path ]]; then
    path=$(pwd)
  fi
  paths[$parameter]="$path"
  echo "Path add: $parameter -> $path"
}
# adls command
adls() {
  echo "Path added:"
  for key in "${!paths[@]}"; do
    echo "$key -> ${paths[$key]}"
  done
}
# adcl command
adcl() {
  local parameter=$1
  if [[ -n ${paths[$parameter]} ]]; then
    unset paths[$parameter]
    echo "Path deleted: $parameter"
  else
    echo "Parameter not found: $parameter"
  fi
}

