first make sure u did all docker stuff

first do:

gcc -o ttt-client client.c pdu.c
gcc -o ttt-server server.c pdu.c game.c users.c

and then do:

./ttt-client dakshesh 127.0.0.1 15464 (tyler, replace w ur computer username)
docker-compose run --rm ref-client test_user host.docker.internal 15464 

