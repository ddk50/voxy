#!/bin/bash

function vm1_start {
	ssh 172.29.4.111 qemu -vnc 0.0.0.0:1 -daemonize -pidfile ~/vm1.pid \
		-m 256 -cdrom ~/Common/Download/install-x86-minimal-20110111.iso
    echo "VM1 Start"
}

function vm1_stop {
	ssh 172.29.4.111 "kill \`cat ~/vm1.pid\`"
    echo "VM1 Stop"
}

function vm2_start {
	ssh 172.29.4.111 qemu -vnc 0.0.0.0:2 -daemonize -pidfile ~/vm2.pid \
		-m 256 -cdrom ~/Common/Download/install-x86-minimal-20110111.iso
    echo "VM2 Start"
}

function vm2_stop {
	ssh 172.29.4.111 "kill \`cat ~/vm2.pid\`"
    echo "VM2 Stop"
}


# invoke function
$1

