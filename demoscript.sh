g++ retriever.cpp -o retriever
g++ server.cpp -o server -lpthread
./server &

./retriever www.google.com/
./retriever 127.0.0.1/
./retriever 127.0.0.1/SecretFile.html
./retriever 127.0.0.1/../passwds.html
./retriever 127.0.0.1/fubar.png
./retriever 127 0.0.1/

killall server
