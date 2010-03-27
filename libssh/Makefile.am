AM_CFLAGS = -O2 -pipe -Wall @ENABLE_DEBUG@
AM_CPPFLAGS = -DWITH_SERVER -DWITH_SSH1 -DWITH_SFTP -I../include

lib_LTLIBRARIES = libfbssh.la
libfbssh_la_SOURCES = agent.c auth.c auth1.c base64.c buffer.c callbacks.c \
                      channels.c channels1.c client.c config.c connect.c \
					  crc32.c crypt.c dh.c error.c gcrypt_missing.c gzip.c \
					  init.c kex.c keyfiles.c keys.c log.c match.c messages.c \
					  misc.c options.c packet.c pcap.c poll.c scp.c server.c \
					  session.c sftp.c sftpserver.c socket.c string.c wrapper.c

