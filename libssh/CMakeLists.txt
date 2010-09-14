add_definitions(-DWITH_SERVER -DWITH_SSH1 -DWITH_SFTP)

add_library(fbssh SHARED agent.c auth.c auth1.c base64.c buffer.c callbacks.c
	channels.c channels1.c client.c config.c connect.c crc32.c crypt.c dh.c
	error.c gcrypt_missing.c gzip.c init.c kex.c keyfiles.c keys.c log.c 
	match.c messages.c misc.c options.c packet.c pcap.c poll.c scp.c server.c
	session.c sftp.c sftpserver.c socket.c string.c wrapper.c)

install(TARGETS fbssh
	RUNTIME DESTINATION so)
