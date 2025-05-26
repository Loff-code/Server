#!/bin/sh

apt update
apt upgrade -y

apt-get install -y git
apt-get install -y libssl-dev
apt-get install -y build-essential

echo "bootstrap.sh done"

