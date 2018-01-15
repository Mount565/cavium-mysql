#!/bin/sh

cd /build
cp /source/packaging/rpm-common/filter-{provides,requires}.sh .
curl -sSOL https://github.com/mysql/mysql-server/archive/mysql-5.6.37.tar.gz
