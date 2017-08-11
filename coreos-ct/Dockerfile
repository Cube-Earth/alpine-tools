FROM golang:alpine
#ENV CGO_ENABLED=0  --> replaced by modified LDFLAGS and CC/gcc, because disabling CGO seems to have several impacts

ENV CT_VER=0.3.1

RUN apk add --update jq ca-certificates gnupg bash sudo abuild gcc
RUN addgroup sudo && \
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
	adduser -D -g '' -G abuild -s /sbin/nologin build && \
	adduser build sudo && \
	mkdir -p /apk_build && \
	chgrp -R abuild /apk_build /go && \
	chmod -R g+w /apk_build /go

USER build
WORKDIR /apk_build

COPY files/* /apk_build/

VOLUME /apk_keys
VOLUME /apk

ENTRYPOINT ["./build.sh"]
