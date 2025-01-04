#!/bin/bash
dir=$(pwd)
if [ $# == 2 ]; then
    if [ -f "$dir/instrumentation.sh" ]; then
        if [[ $1 == *"dns"* ]]; then
            target_ll=../tests/dnsmasq-2.78/src/dnsmasq.ll
            configfile="../Result/dnsmasq/config/sel$2"

            echo "opt -load ./src/build/libInstrumentation.so -gadgetattach -S $target_ll -config-path=$configfile -o ../Result/dnsmasq/dnsmasq_new.ll"

            opt -load ./src/build/libInstrumentation.so -gadgetattach -S $target_ll -config-path=$configfile -o ../Result/dnsmasq/dnsmasq_new.ll
            llc ../Result/dnsmasq/dnsmasq_new.ll -filetype=obj -o ../Result/dnsmasq/dnsmasq_new.o -O0
            gcc ../Result/dnsmasq/dnsmasq_new.o -o ../Result/dnsmasq/dnsmasq -Wall -W -O0 -g -fno-stack-protector -m32 -no-pie
        elif [[ $1 == *"latex"* ]]; then
            target_ll=../tests/latex2rtf-1.9.15/latex2rtf.ll
            configfile="../Result/latex2rtf/config/sel$2"

            echo "opt -load ./src/build/libInstrumentation.so -gadgetattach -S $target_ll -config-path=$configfile -o ../Result/latex2rtf/latex2rtf_new.ll"

            opt -load ./src/build/libInstrumentation.so -gadgetattach -S $target_ll -config-path=$configfile -o ../Result/latex2rtf/latex2rtf_new.ll
            llc ../Result/latex2rtf/latex2rtf_new.ll -filetype=obj -o ../Result/latex2rtf/latex2rtf_new.o -O0
            gcc ../Result/latex2rtf/latex2rtf_new.o -o ../Result/latex2rtf/latex2rtf -Wall -W -O0 -g -fno-stack-protector -m32 -no-pie
        fi
    fi
else
    echo "需要两个参数：target、conf_num"
fi
