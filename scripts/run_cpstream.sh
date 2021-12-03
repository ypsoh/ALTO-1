#!/bin/bash


LOG_DIR="/logs"

TENSORS=(
  lbnl 
  nips 
  uber 
  chicago-crime-comm 
  vast 
  # darpa 
  enron 
  # nell-2 
  # fb-m 
  flickr-4d 
  delicious-4d 
  # nell-1 
  # amazon
)

get_tns_name() {
  declare -A tns_files=(
    ["lbnl"]="lbnl"
    ["nips"]="nips"
    ["uber"]="uber"
    ["chicago"]="chicago-crime-comm"
    ["vast"]="vast-2015-mc1-5d"
    ["enron"]="enron"
    ["flickr-4d"]="flickr-4d" 
    ["delicious-4d"]="deicious-4d"
  )
  echo "${tns_files[${1}]}.tns"
}

get_streaming_mode() {
  declare -A streaming_mode=(
    ["lbnl"]="4"
    ["nips"]="3"
    ["uber"]="0"
    ["chicago"]="0"
    ["vast"]="0"
    ["enron"]="0"
    ["flickr-4d"]="3" 
    ["delicious-4d"]="3" 
  )
  echo "${streaming_mode[${1}]}"
}

VARIATIONS=(
  spcpstream 
  spcpstream_alto 
  cpstream_alto 
  cpstream
)

# for ((i=0; i < ${#TENSORS[@]}; ++i))
# do
#   for ((v=0; v < ${#VARIATIONS[@]}; ++v))
#   do
#     # Replace fields in template.srun file and submit job
#     ./cpd128 --rank 16 -m 300 -a ${STREAMING_MODE[i]} -e 1e-5 -l ${VARIATIONS[v]} -x 44 -i ~/hpctensor/${TENSORS[i]}.tns > ${TENSORS[i]}.${VARIATIONS[v]}.log.txt 2>&1 
#   done
# done

dataset="chicago"
TNS_FILE=$(get_tns_name $dataset)
STREAMING_MODE=$(get_streaming_mode $dataset)

echo "Running ${TNS_FILE}-streaming mode ${STREAMING_MODE}"