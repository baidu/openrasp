#!/bin/bash

docker run --name sqlserver -d -e ACCEPT_EULA=Y -e SA_PASSWORD="yourStrong(!)Password" -p 1433:1433 microsoft/mssql-server-linux:2017-GA

docker run --name oracle -d -e ORACLE_ALLOW_REMOTE=true -p 1521:1521 alexeiled/docker-oracle-xe-11g

# docker run -d -e MYSQL_ROOT_PASSWORD="my-secret-pw" -p 3306:3306 mysql:5

# docker run -d -e POSTGRES_PASSWORD="mysecretpassword" -p 5432:5432 postgres:9-alpine