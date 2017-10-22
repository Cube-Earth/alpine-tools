FROM cubeearth/oracle-java7-server

ENV TOMCAT_APR_VER=1.2.14

RUN apk add --update jq ca-certificates gnupg bash sudo abuild gcc
RUN addgroup sudo && \
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
	adduser -D -g '' -G abuild -s /sbin/nologin build && \
	adduser build sudo && \
	mkdir -p /apk_build && \
	chgrp -R abuild /apk_build && \
	chmod -R g+w /apk_build

USER build
WORKDIR /apk_build

COPY files/* /apk_build/

VOLUME /apk_keys
VOLUME /apk

ENTRYPOINT ["./build.sh"]
