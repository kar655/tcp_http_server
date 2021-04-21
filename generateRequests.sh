#!/bin/bash

#echo -ne "GET /file.txt HTTP/1.1\r\nfirst:123   \r\nsecondeeeeee: 2137\r\n\r\n"
##
##sleep 1
#
#echo -ne "GET /a.txt HTTP/1.1\r\nonlyThis: 2137\r\n\r\n"

#echo -e "HEAD directory2137/file HTTP/1.1\x0d\x0a"

echo -ne "GET /file.txt HTTP"
sleep 1
echo -ne "/1.1\r\nfirst:123   \r\nseconde"
sleep 1
echo -ne "eeeee: 2137\r\n\r\n"

exit 0
