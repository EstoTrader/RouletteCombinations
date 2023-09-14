//Header que prototipa  las funciones de mi framwwork para trabajar con SQLite3
//Las funciones ya prototipadas las desarroyo en midbinterface.cpp

//COPIE DE AQUI COMO HACER UN HEADER https://stackoverflow.com/questions/15891781/how-to-call-on-a-function-found-on-another-file

#ifndef MIDBINTERFACE_H // esto asegura que no declaras las funciones mas de una vez
#define MIDBINTERFACE_H

#include <stdio.h>
#include <iostream>
#include "sqlite3.h" //Las comillas en este include son por que el header va en el mismo directorio que el .cpp
#include <string>
#include <vector>

void holamundo();

static int callback(void* data, int argc, char** argv, char** azColName);
void db_crear(const char* db_nombre);
void db_abrir(const char* db_nombre);
void db_cerrar(const char* db_nombre);
void db_meter_vectores(const char* db_nombre, std::vector<long long int>& Id,
	std::vector <int>& rn1, std::vector <int>& rn2, std::vector <int>& rn3,
	std::vector <int>& pi1, std::vector <int>& pi2, std::vector <int>& pi3,
	std::vector <int>& pf1, std::vector <int>& pf2, std::vector <int>& pf3
	);



#endif // final de la cabecera para que la reconozca