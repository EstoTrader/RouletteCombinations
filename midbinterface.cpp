#include "midbinterface.h"

//vector donde voy a meter los int del callback
std::vector<int> callback_vector;


static int callback(void* data, int argc, char** argv, char** azColName) {
    std::cout << "DEBUG LLAMADO CALLBACK DE midbinterface" << "\n";

    // es un callbak pero enfocado a como si hicieramos select en el querry
    int i;
    //fprintf(stderr, "%s: ", (const char*)data);

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        //pruebas para sacar solo el numero
        std::cout << argv[i] <<"\n"; //este es el que saca en numero
    }

    

    printf("\n");
    return 0;
}
static int callback_NEW(void* data, int argc, char** argv, char** azColName) {
    // es un callbak pero enfocado a como si hicieramos select en el querry
    int i;
    std::string str;
    int numeroconvertido;
    //fprintf(stderr, "%s: ", (const char*)data);

    //borro vector callback
    callback_vector.clear();

    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");


        //asigno el numero extraido que es un char a un int, siguiendo las 2 lineas de abajo
        //const char* testaa = "23";
        //std::string str(testaa)

        std::string str(argv[i]);
        numeroconvertido = std::stoi(str);
        std::cout << "numero convertido es: " << numeroconvertido << "\n";
        //pruebas para sacar solo el numero
        //std::cout << argv[i] << "\n"; //este es el que saca en numero
        
        //mento el numero en el vector de vuelta
        callback_vector.push_back(numeroconvertido);

    }

    printf("\n");
    return 0;
   
}

void db_crear(const char* db_nombre) {
	//Creo data base en el mismo directorio con el nombre que medan
     //SACADO DE https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
   


    // Pointer to SQLite connection
    sqlite3* db;

    // Save any error messages
    char* zErrMsg = 0;

    // Save the result of opening the file
    int rc;

    // Save any SQL
    std::string sql;

    const char* data = "Callback function called";

    // Save the result of opening the file    
    rc = sqlite3_open(db_nombre, &db);

    if (rc) {
        // Show an error message
        std::cout << "DB Error: " << sqlite3_errmsg(db) << std::endl;
        // Close the connection
        sqlite3_close(db);
        // Return an error
        //return(1);
    }

    // Save SQL to create a table
    sql = "CREATE TABLE SA ("  \
        "id INT PRIMARY KEY     NOT NULL," \
        "rn1         INT    NOT NULL,"\
        "rn2         INT    NOT NULL,"\
        "rn3         INT    NOT NULL,"\
        "pi1         INT    NOT NULL,"\
        "pi2         INT    NOT NULL,"\
        "pi3         INT    NOT NULL,"\
        "pf1         INT    NOT NULL,"\
        "pf2         INT    NOT NULL,"\
        "pf3         INT    NOT NULL);";


    /* Open database, a ver si abriendola no da problemas PUEDE QUE SOBRE por estar ya creada */
    rc = sqlite3_open(db_nombre, &db);

    // Run the SQL (convert the string to a C-String with c_str() )
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //meterdatos

    sql = "REPLACE INTO SA VALUES (100200300,1,2,3,3,4,5,6,7,8,9);";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //veo meto un indice con numero tocho
    long long int dbl01 = 1010101020202030303;
    sql = "REPLACE INTO SA VALUES (" + std::to_string(dbl01) + ", 133, 233, 333); ";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
  

    // Close the SQL connection
    sqlite3_close(db);

    //return 0;
}
void db_abrir(const char* db_nombre) {
	//Abro la database con el nombre que me dan
}
void db_cerrar(const char* db_nombre)
{
	//Cierro la database con el numbre que me dan
}
void db_meter_vectores(const char* db_nombre, std::vector<long long int>& Id,
	std::vector <int>& rn1, std::vector <int>& rn2, std::vector <int>& rn3,
	std::vector <int>& pi1, std::vector <int>& pi2, std::vector <int>& pi3,
	std::vector <int>& pf1, std::vector <int>& pf2, std::vector <int>& pf3) {
	//dados 1 vector que es la Id de la data base, 6 vectores más que son el numero de grupos de repeticiones detectadas
	//meto en la base de datos los nuevos hayados y los sumo a lo que ya está si es que hay algo

	//abro la database con el nombre que me dan

	for (int index = 0; index < Id.size(); ++index)
	{
		
	
	//leo los datos que me interesan de la posicion 
	

	//sumo los datos de los vectores a los de la data base // access vector using [index]
	

	//actualizo los datos en la database // access vector using [index]

	}

	//cierro la data base
}

void holamundo()
{
std::cout << "holamundo";
}