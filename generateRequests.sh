#!/bin/bash

echo -ne "GET /file.txt HTTP/1.1\r\n"
echo -ne "first:123   \r\n"
echo -ne "secondeeeeee: 2137\r\n\r\n"

sleep 1

echo -ne "GET /a.txt HTTP/1.1\r\n"
echo -ne "onlyThis: 2137\r\n\r\n"

#echo -e "HEAD directory2137/file HTTP/1.1\x0d\x0a"

exit 0
