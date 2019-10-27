all: custom_storage.a
	echo "OK"

custom_storage.a:
	g++ -c custom_storage.cpp -std=c++11
	ar -rcs libcustom_storage_octosql.a custom_storage.o
