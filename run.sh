#!/bin/bash

module load intel/19
source /packages/intel/19/linux/pkg_bin/compilervars.sh -arch intel64 -platform linux
# make && ./cpd64 --rank 16 -m 100 -a 0 -i ~/hpctensor/uber.tns
# make && ./cpd64 --rank 16 -m 100 -a 0 -i ~/hpctensor/chicago-crime-comm.tns
# make && ./cpd64 --rank 16 -m 100 -a 3 -i ~/hpctensor/flickr-4d.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -x 44 -i ~/hpctensor/sm_flickr.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -x 44 -i ~/hpctensor/flickr-4d.tns
# make && ./cpd64 --rank 16 -m 100 -i ~/hpctensor/sm_flickr.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -i ~/hpctensor/flickr-4d.tns

# make && ./cpd64 --rank 16 -m 100 -a 0 -i ~/hpctensor/uber.tns
# make && ./cpd64 --rank 16 -m 100 -a 0 -i ~/hpctensor/chicago-crime-comm.tns
# make && ./cpd64 --rank 16 -m 100 -a 3 -i ~/hpctensor/flickr-4d.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream -x 44 -i ~/hpctensor/sm_flickr.tns
# make && ./cpd128 --rank 16 -m 100 -a 0 -l cpstream -x 44 -i ~/hpctensor/uber.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream -x 44 -i ~/hpctensor/sm_flickr.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l spcpstream -x 44 -i ~/hpctensor/sm_flickr.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l spcpstream -x 44 -i ~/hpctensor/nips.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream -x 44 -i ~/hpctensor/nips.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream_alto -x 44 -i ~/hpctensor/nips.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream -x 44 -i ~/hpctensor/sm_flickr.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream -x 44 -i ~/hpctensor/sm_flickr.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream_alto -x 44 -i ~/hpctensor/sm_flickr.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -l spcpstream -x 44 -i ~/hpctensor/nips.tns
# make && ./cpd64 --rank 16 -m 100 -a 0 -i ~/hpctensor/uber.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l cpstream -x 44 -i ~/hpctensor/delicious-4d.tns
#./cpd128 --rank 16 -m 100 -a 0 -l cpstream_alto -x 44 -i ~/hpctensor/chicago-crime-comm.tns
# make && ./cpd128 --rank 16 -m 100 -a 3 -i ~/hpctensor/delicious-4d.tns

#uber
#make && ./cpd128 --rank 16 -m 200 -e 1e-5 -a 0 -l spcpstream_alto -x 44 -i ~/hpctensor/uber.tns
#make && ./cpd128 --rank 16 -m 200 -e 1e-3 -a 0 -l cpstream -x 44 -i ~/hpctensor/uber.tns
#make && ./cpd128 --rank 16 -m 200 -a 0 -e 1e-3 -a 0 -l cpstream -x 44 -i ~/hpctensor/uber.tns

#flickr
#make && ./cpd128 --rank 16 -m 100 -a 3 -l spcpstream_alto -x 44 -i ~/hpctensor/flickr-4d.tns

#debug
#make && ./cpd128 --rank 16 -m 100 -a 3 -l spcpstream -x 44 -i ~/hpctensor/sm_flickr.tns
#make && ./cpd128 --rank 16 -m 100 -a 3 -l spcpstream_alto -x 44 -i ~/hpctensor/sm_flickr.tns

#make && ./cpd128 --rank 16 -m 200 -e 1e-5 -l als -x 44 -i ~/hpctensor/uber.tns
#make && ./cpd128 --rank 16 -m 200 -e 1e-5 -l als -x 44 -i ~/hpctensor/flickr-4d.tns


if [ -z "$1" ]
then
  make && ./cpd128 --rank 16 -m 200 -a 3 -e 1e-3 -l cpstream -x 44 -i ~/hpctensor/sm_flickr.tns
else
  make && ./cpd128 --rank 16 -m 200 -a 3 -e 1e-3 -l cpstream -z nonneg -x 44 -i ~/hpctensor/sm_flickr.tns
fi
