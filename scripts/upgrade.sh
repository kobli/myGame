#!/bin/bash
branch=<BRANCH>
arch=<ARCH>
remote_addr=http://kobl.eu/dl/mygame/packages/$branch
tag=`curl $remote_addr/latest`
current_tag=`cat version`
file_name=myGame-client-$tag-$arch.zip
remote_file_addr=$remote_addr/$tag/$file_name
config_backup_dir=config_backup/$current_tag

if [ $tag = $current_tag ]; then
	echo "Already up to date"
else
	mkdir -p $config_backup_dir
	mv config/* $config_backup_dir

	for f in *; do
		[ "$f" = "config_backup" ] && continue
		rm -rf "$f"
	done
	curl -O $remote_file_addr
	unzip $file_name
fi
