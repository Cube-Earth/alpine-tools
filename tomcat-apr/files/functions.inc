#!/bin/sh

GITHUB_RELEASE_NAME='repository/x86_64'
GITHUB_RELEASE_URI=`echo $GITHUB_RELEASE_NAME | sed 's#/#%2f#g'`


function show_error
{
  echo "ERROR: $1"
  exit 1
}

function check
{
	if [[ -z "`eval echo \\$$1`" ]]
	then
		echo "ERROR: environment variable $1 not set!"
		exit 1
	fi
}

function create_key
{
	local f="/apk_keys/$GITHUB_EMAIL.rsa"
	if [[ ! -f "$f" ]]
	then
		echo "$f" | abuild-keygen -ia
	else
		mkdir -p ~/.abuild
		cat > ~/.abuild/abuild.conf << EOF
PACKAGER_PRIVKEY="$f"
EOF
		sudo cp "$f.pub" /etc/apk/keys
	fi
}

function create_release
{
	if ! curl -I -X HEAD -s -f -u "$GITHUB_EMAIL:$GITHUB_PWD" https://api.github.com/repos/$GITHUB_REPO/releases/tags/$GITHUB_RELEASE_URI > /dev/null 2>&1
	then
		echo "creating release $GITHUB_RELEASE_NAME ..."
		curl -X POST -T - -u "$GITHUB_EMAIL:$GITHUB_PWD" https://api.github.com/repos/$GITHUB_REPO/releases << EOF
{
  "tag_name": "$GITHUB_RELEASE_NAME",
  "target_commitish": "master",
  "name": "$GITHUB_RELEASE_NAME",
  "body": "APK repository for Alpine tools (x86_64)",
  "draft": false,
  "prerelease": true
}
EOF
	fi
}

function get_asset_id
{
	local name="$1"
	local id=`curl -s -u "$GITHUB_EMAIL:$GITHUB_PWD" https://api.github.com/repos/$GITHUB_REPO/releases/tags/$GITHUB_RELEASE_URI 2>/dev/null | jq --arg v "$name" -r '.assets[] | select(.name==$v or .label==$v) | .id'`
	echo $id
}

function delete_asset
{
	[[ -z "$1" ]] || curl -X DELETE -u "$GITHUB_EMAIL:$GITHUB_PWD" https://api.github.com/repos/$GITHUB_REPO/releases/assets/$1
}

function upload_asset
{
	local file="$1"
	local name="$2"
	local label="$3"
	
	[[ -z "$file" ]] && show_error "missing file name for upload!"
	[[ -z "$name" ]] && name=`basename "$file"`
	[[ -z "$label" ]] && label="$name"
	
	echo ""
	echo "--- $name ------------------------------"	
	
	local UPLOAD=`curl -s -u "$GITHUB_EMAIL:$GITHUB_PWD" https://api.github.com/repos/$GITHUB_REPO/releases/tags/$GITHUB_RELEASE_URI | jq -r '.upload_url' | sed 's/{.*}$//'`

	curl -X POST -T "$file" --header "Content-length: `stat -c \"%s\" $file`" --header "Content-type: application/gzip" -u "$GITHUB_EMAIL:$GITHUB_PWD" "$UPLOAD?name=$name&label=$label"
}

function replace_asset
{
	local file="$1"
	local name="$2"
	local label="$3"
	
	[[ -z "$file" ]] && show_error "missing file name for upload!"
	[[ -z "$name" ]] && name=`basename "$file"`
	[[ -z "$label" ]] && label="$name"

	delete_asset `get_asset_id "$name"`
	upload_asset "$file" "$name" "$label"
}

function upload_new
{
	local file="$1"
	local name="$2"
	local label="$3"
	
	[[ -z "$file" ]] && show_error "missing file name for upload!"
	[[ -z "$name" ]] && name=`basename "$file"`
	[[ -z "$label" ]] && label="$name"
	
	local id=`get_asset_id "$name"`
	[[ -z "$id" ]] && upload_asset "$file" "$name" "$label"
}

function prepare
{
	check GITHUB_EMAIL
	check GITHUB_REPO
	check GITHUB_PWD
	
	create_key
}

function replace_assets
{
	create_release

	upload_new "/apk_keys/$GITHUB_EMAIL.rsa.pub"

	replace_asset /apk/x86_64/APKINDEX.tar.gz

	IFS=$'\n'
	for f in `cat ~/apk_names`
	do
		replace_asset /apk/x86_64/$f
	done
	
	echo ""
}
