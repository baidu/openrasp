<?php
$mysql_host       = getenv("MYSQL_TEST_HOST")     ? getenv("MYSQL_TEST_HOST")      : "127.0.0.1";
$mysql_port       = getenv("MYSQL_TEST_PORT")     ? getenv("MYSQL_TEST_PORT")      : 3306;
$mysql_user       = getenv("MYSQL_TEST_USER")     ? getenv("MYSQL_TEST_USER")      : "test";
$mysql_passwd     = getenv("MYSQL_TEST_PASSWD")   ? getenv("MYSQL_TEST_PASSWD")    : "test";
$mysql_db         = getenv("MYSQL_TEST_DB")       ? getenv("MYSQL_TEST_DB")        : "test";
$mysql_table      = getenv("MYSQL_TEST_TABLE")    ? getenv("MYSQL_TEST_TABLE")     : "vuln";

$pg_host          = getenv("PGSQL_TEST_HOST")     ? getenv("PGSQL_TEST_HOST")      : "127.0.0.1";
$pg_port          = getenv("PGSQL_TEST_PORT")     ? getenv("PGSQL_TEST_PORT")      : 5432;
$pg_user          = getenv("PGSQL_TEST_USER")     ? getenv("PGSQL_TEST_USER")      : "test";
$pg_passwd        = getenv("PGSQL_TEST_PASSWD")   ? getenv("PGSQL_TEST_PASSWD")    : "test";
$pg_db            = getenv("PGSQL_TEST_DB")       ? getenv("PGSQL_TEST_DB")        : "test";
$pg_table         = getenv("PGSQL_TEST_TABLE")    ? getenv("PGSQL_TEST_TABLE")     : "vuln";
?>