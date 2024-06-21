#!/bin/bash
# 存储路径的数组
declare -A paths
# go命令
go() {
  local parameter=$1
  local path=${paths[$parameter]}

  if [[ -n $path ]]; then
    cd "$path" || echo "无法跳转到路径: $path"
  else
    echo "未找到参数: $parameter"
  fi
}
# adir命令
adir() {
  local parameter=$1
  local path=$2
 if [[ -z $path ]]; then
    path=$(pwd)
  fi
  paths[$parameter]="$path"
  echo "路径已添加: $parameter -> $path"
}
# adls命令
adls() {
  echo "已添加的路径:"
  for key in "${!paths[@]}"; do
    echo "$key -> ${paths[$key]}"
  done
}
# adcl命令
adcl() {
  local parameter=$1
  if [[ -n ${paths[$parameter]} ]]; then
    unset paths[$parameter]
    echo "路径已删除: $parameter"
  else
    echo "未找到参数: $parameter"
  fi
}

# 主循环
while true; do
  read -p "> " command
  case $command in
    go*)
      go "${command:3}"
      ;;
    adir*)
      adir "${command:4}"
      ;;
    adls)
      adls
      ;;
    adcl*)
      adcl "${command:4}"
      ;;
    exit)
      break
      ;;
  esac
done
