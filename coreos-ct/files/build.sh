#!/bin/sh
. ./functions.inc

prepare

abuild -r -P /apk deps build checksum rootpkg index || exit 1

replace_assets
