#!/bin/bash
HASH_FUNC=md5sum
RESOURCES_DIR=./cached_files # source
DOWNLOADS_DIR=./courses/ud923/filecorpus # dest

for f in `ls -1 $DOWNLOADS_DIR`
do
    r=`$HASH_FUNC $RESOURCES_DIR/$f`; echo $r
    d=`$HASH_FUNC $DOWNLOADS_DIR/$f`; echo $d

    rh=`echo $r | cut -d' ' -f1`
    dh=`echo $d | cut -d' ' -f1`
    if [ "x$dh" == "x$rh" ]; then
        echo "$f: passed ($HASH_FUNC)"
    else
        echo "$f: failed ($HASH_FUNC)"
    fi
    echo
done

