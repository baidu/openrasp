#!/bin/bash

set -ex
cd "$(dirname "$0")"/../

xgettext --keyword=_ --language=C --add-comments --sort-output \
	--copyright-holder "Baidu Inc." \
	-c'FIRST AUTHOR'='OpenRASP Developers <openrasp@baidu.com>' \
	--msgid-bugs-address openrasp@baidu.com \
	-j -o po/openrasp.pot $(find . -name '*.cc' -o -name '*.c')

for lang in zh_CN.UTF-8 zh_TW fr_FR es_ES
do
    mkdir -p po/"$lang"
    msginit --no-translator --input=po/openrasp.pot --locale="$lang" --output=po/"$lang"/openrasp.po
done

