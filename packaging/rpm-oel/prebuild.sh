#!/bin/sh

cp /source/packaging/rpm-common/filter-{provides,requires}.sh /build
wget -cP /build https://github.com/mysql/mysql-server/archive/mysql-5.6.37.tar.gz
