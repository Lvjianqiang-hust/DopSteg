#!/bin/bash
dir=$(pwd)
if [ -f "$dir/search.sh" ]; then
    if [[ $1 == *"dns"* ]]; then
        # 目标程序的ll文件，用wllvm编译得到
        target_ll=../tests/dnsmasq-2.78/src/dnsmasq.ll
        # 攻击语义文件
        asdl=../tests/dnsmasq-2.78/src/getPID.ASDL
        # 调度器所在函数
        dispatcher_func=main
        # 发生溢出的函数
        buffer_location=dhcp6_reply
        # 发生溢出的变量
        buffer=state
        # 发生溢出的缓冲区的大小
        buffersize=16
        # 发生溢出的缓冲区
        buffername=state.mac
        # 在发生溢出的函数中的漏洞函数（可能是strcpy，也可能是自定义函数）
        # 后面的数字表示第几个，因为一个函数内可能多次调用同一个函数
        statement=dhcp6_maybe_relay,1

        outfile=../Result/dnsmasq/gadgets
        locfile=../Result/dnsmasq/location
        logfile=../Result/dnsmasq/log

        echo "opt -load ./src/build/libGadgetSearcher.so -search $target_ll -asdl=$asdl \
        -dispatcher=$dispatcher_func -location=$buffer_location -buffer=$buffer -buffersize=$buffersize -buffername=$buffername -statement=$statement \
        -outfile=$outfile -locfile=$locfile -logfile=$logfile -o /dev/null"

        opt -load ./src/build/libGadgetSearcher.so -search $target_ll -asdl=$asdl \
        -dispatcher=$dispatcher_func -location=$buffer_location -buffer=$buffer -buffersize=$buffersize -buffername=$buffername -statement=$statement \
        -outfile=$outfile -locfile=$locfile -logfile=$logfile -o /dev/null

    elif [[ $1 == *"latex"* ]]; then
        target_ll=../tests/latex2rtf-1.9.15/latex2rtf.ll
        asdl=../tests/latex2rtf-1.9.15/getPath.ASDL
        dispatcher_func=ConvertWholeDocument
        buffer_location=expandmacro
        buffer=buffer
        buffersize=1024
        buffername=buffer
        statement=strcpy,3

        outfile=../Result/latex2rtf/gadgets
        locfile=../Result/latex2rtf/location
        logfile=../Result/latex2rtf/log

        echo "opt -load ./src/build/libGadgetSearcher.so -search $target_ll -asdl=$asdl \
        -dispatcher=$dispatcher_func -location=$buffer_location -buffer=$buffer -buffersize=$buffersize -buffername=$buffername -statement=$statement \
        -outfile=$outfile -locfile=$locfile -logfile=$logfile -o /dev/null"

        opt -load ./src/build/libGadgetSearcher.so -search $target_ll -asdl=$asdl \
        -dispatcher=$dispatcher_func -location=$buffer_location -buffer=$buffer -buffersize=$buffersize -buffername=$buffername -statement=$statement \
        -outfile=$outfile -locfile=$locfile -logfile=$logfile -o /dev/null

    elif [[ $1 == *"ftpd"* ]]; then
        # 目标程序的ll文件，用wllvm编译得到
        target_ll=../tests/proftpd-1.3.0/proftpd.ll
        # 攻击语义文件
        asdl=../tests/proftpd-1.3.0/getKey.ASDL
        # 调度器所在函数
        dispatcher_func=cmd_loop
        # 发生溢出的函数
        buffer_location=sreplace
        # 发生溢出的变量
        buffer=buf
        # 发生溢出的缓冲区的大小
        buffersize=4096
        # 发生溢出的缓冲区
        buffername=buf
        # 在发生溢出的函数中的漏洞函数（可能是strcpy，也可能是自定义函数）
        # 后面的数字表示第几个，因为一个函数内可能多次调用同一个函数(序号从1开始，这里表示在IR代码中遇到的第一个sstrncpy)
        statement=sstrncpy,1

        outfile=../Result/proftpd/gadgets
        locfile=../Result/proftpd/location
        logfile=../Result/proftpd/log

        echo "opt -load ./src/build/libGadgetSearcher.so -search $target_ll -asdl=$asdl \
        -dispatcher=$dispatcher_func -location=$buffer_location -buffer=$buffer -buffersize=$buffersize -buffername=$buffername -statement=$statement -range=ALL \
        -outfile=$outfile -locfile=$locfile -logfile=$logfile -o /dev/null"

        opt -load ./src/build/libGadgetSearcher.so -search $target_ll -asdl=$asdl \
        -dispatcher=$dispatcher_func -location=$buffer_location -buffer=$buffer -buffersize=$buffersize -buffername=$buffername -statement=$statement -range=ALL \
        -outfile=$outfile -locfile=$locfile -logfile=$logfile -o /dev/null
        python3 ./graph-process-python/filter.py
    fi
fi

