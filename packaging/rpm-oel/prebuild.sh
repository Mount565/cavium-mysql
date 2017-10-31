#!/bin/sh

cp /source/packaging/rpm-common/filter-{provides,requires}.sh /build
wget -cP /build https://cdn.mysql.com/Downloads/MySQL-5.6/mysql-5.6.37.tar.gz
