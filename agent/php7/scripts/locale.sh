#!/bin/bash

set -ex
cd "$(dirname "$0")"/../

locale_list="zh_CN.utf8 zh_TW fr_FR es_ES"

if test -f po/openrasp.pot 
then
jeoption=" -j "
fi

xgettext --keyword=_ --language=C --add-comments --sort-output \
	--copyright-holder "Baidu Inc." \
	-c'FIRST AUTHOR'='OpenRASP Developers <openrasp@baidu.com>' \
	--msgid-bugs-address openrasp@baidu.com \
	$jeoption -o po/openrasp.pot $(find . -name '*.cc' -o -name '*.c')

for lang in $locale_list
do
	if !(test -d po/"$lang"/LC_MESSAGES)
	then
		mkdir -p po/"$lang"/LC_MESSAGES
	fi

	if test -f po/"$lang"/openrasp.po
	then
		msgmerge --update po/"$lang"/openrasp.po po/openrasp.pot
	else
		msginit --no-translator --input=po/openrasp.pot --locale="$lang" --output=po/"$lang"/openrasp.po
	fi

	msgfmt --output-file=po/"$lang"/LC_MESSAGES/openrasp.mo po/"$lang"/openrasp.po
done

cd po
tar cvf locale.tar `find . -name  \*.mo`
cd ..

for lang in $locale_list
do
	rm -rf po/"$lang"/LC_MESSAGES
done

