#define _CRT_SECURE_NO_WARNINGS //para que deje compilar funciones de tiempo
#include <stdio.h>
#include <iostream>
#include "sqlite3.h" //Las comillas en este include son por que el header va en el mismo directorio que el .cpp
//#include "midbinterface.h" //es una especie de framework que me cree para manejar sqlite
#include <string>
#include <vector>
#include <typeinfo> //para saber el tipoi de las variables
#include <sstream>      // std::istringstream
#include <cstdlib> //para numero random
#include <ctime>  //para numero random
#include <chrono> //para calcular tiempo de procesos
#include <iomanip> //put_time
#include <sstream> //string stream, lo uso en chrono

using namespace std;

//20220801 Cambios que se van a meter en esta version// 
//Pasar todos los campos de vectores temp_XX, sa_XX y cb_XX a long long int
//Cambios en la base de datos y nomenclatura del index
// 
//index es abbcddddeffffghhhh donde a es siempre 1, bb es el numero de bolas que llevamos
//c es la ultima repeticion detectada en el grupo RN , dddd es el numero de reps RN dd(reps1)d(reps2)d(reps3)
//e es la ultima repeticion detectada en el grupo PI , ffff es el numero de reps PI ff(reps1)f(reps2)f(reps3)
//g es la ultima repeticion detectada en el grupo PF , hhhh es el numero de reps PF hh(reps1)h(reps2)h(reps3)
// 
//añadir vectores ID_reps OKOK
//

//campos de base de datos se añade NV que es el numero de veces que se ha dado es combinacion de id
//quedaria: ID|IDREPS(NUMERO DE VECES QUE SALE EL ID, ESTE ES EL NUEVO)|RN1|RN2|RN3|PI1|PI2|PI3|PF1|PF2|PF3
// ESTO IMPLICA LA CREACION DE BASE DE DATOS CON UN CAMPO MAS, Y EL CALLBACK CON UN CAMPO MAS TB, Y LA LLAMADA AL CALLBACK
//
//cambiar forma de pasar el array creado a base de datos
// ya no se mete a base de datos cuando se termina cada dia sino, cuando de termina toda la simulacion
//1-el temporal del dia temp_XX se pasa al sa_XX
//2-cuando de terminan todos los dias, se busca el cb_XX de la base de datos, y se actualiza la base de datos





//DECLARACIONES DE VARIABLES Y VECTORES
// 
//vectores que acumularán la tabla temp (temporal del Super Array), se simula un array y que se borra en cada sesion despues de 
//pasarlo a los vectores sa
//  tp_ID             sa_RN1 sa_RN2 sa_RN3  sa_PI1 sa_PI2 sa_PI3  sa_PF1 sa_PF2 sa_PF3
// long long int      int    int    int     int    int    int     int    int    int  
//index en base a reps actuales         proximo cambio en el index
std::vector<long long int>temp_ID; std::vector<long long int>temp_IDREPS;
std::vector<long long int>temp_RN1; std::vector<long long int>temp_RN2; std::vector<long long int>temp_RN3;
std::vector<long long int>temp_PI1; std::vector<long long int>temp_PI2; std::vector<long long int>temp_PI3;
std::vector<long long int>temp_PF1; std::vector<long long int>temp_PF2; std::vector<long long int>temp_PF3;



//vectores que acumularán la tabla SA (Super Array), se simula un array
//  sa_ID             sa_RN1 sa_RN2 sa_RN3  sa_PI1 sa_PI2 sa_PI3  sa_PF1 sa_PF2 sa_PF3
// long long int      int    int    int     int    int    int     int    int    int  
//index en base a reps actuales         proximo cambio en el index
std::vector<long long int>sa_ID; std::vector<long long int>sa_IDREPS;
std::vector<long long int>sa_RN1; std::vector<int>sa_RN2; std::vector<int>sa_RN3;
std::vector<long long int>sa_PI1; std::vector<long long int>sa_PI2; std::vector<long long int>sa_PI3;
std::vector<long long int>sa_PF1; std::vector<long long int>sa_PF2; std::vector<long long int>sa_PF3;

//vectores en los que se meterá el callback_masivo
//  cb_ID             sa_RN1 sa_RN2 sa_RN3  sa_PI1 sa_PI2 sa_PI3  sa_PF1 sa_PF2 sa_PF3
// long long int      int    int    int     int    int    int     int    int    int  
//index en base a reps actuales         proximo cambio en el index
std::vector<long long int>cb_ID; std::vector<long long int>cb_IDREPS;
std::vector<long long int>cb_RN1; std::vector<long long int>cb_RN2; std::vector<long long int>cb_RN3;
std::vector<long long int>cb_PI1; std::vector<long long int>cb_PI2; std::vector<long long int>cb_PI3;
std::vector<long long int>cb_PF1; std::vector<long long int>cb_PF2; std::vector<long long int>cb_PF3;

//CREO VECTORES HISTORICOS QUE GUARDAN LAS BOLAN QUE SALEN SON 3 VECTORES, PARA COLOR, PAR Y PASA   
std::vector<int> hist_RN; // 0 si es Rojo 1 si es Negro
std::vector<int> hist_PI; // 0 si es Par 1 si es Impar
std::vector<int> hist_PF; // 0 si es Pasa 1 si es Falta


//vector donde voy a meter los int del callback de la database
std::vector<int> cb_vector; //Call_Back vector

//VARIABLES GLOBALES
long long int ID_actual;
long long int ID_nueva;
int bola_n; //es el contador de bolas tiradas
//int cambio_RN; int cambio_PI; int cambio_PF;
double contador_hito;
double contador_uno_porciento;
double contador_porciento_actual;

//CONSTANTES GLOBALES
long long int  DIAS = 100000;
int BOLAS = 8;
const char* DB_NOMBRE = "db.db";



//Funciones debug
void print_10_vectores_temp()
//imprime los 2 vectores como si estubiesemos haciendo un querry a la base de datos
{
    if (temp_ID.size() > 0)
    {
        std::cout << "DebugPrint 10 Vectores temp_XXX :" << "\n";
        for (int i = 0; i < temp_ID.size(); i++)
        {
            std::cout << temp_ID[i];
            std::cout << "|" << temp_IDREPS[i];
            std::cout << "|" << temp_RN1[i] << "|" << temp_RN2[i] << "|" << temp_RN3[i];
            std::cout << "|" << temp_PI1[i] << "|" << temp_PI2[i] << "|" << temp_PI3[i];
            std::cout << "|" << temp_PF1[i] << "|" << temp_PF2[i] << "|" << temp_PF3[i];
            std::cout << "\n";
        }


    }
    else
    {
        std::cout << "DebugPrint 10 Vectores :" << "\n";
        std::cout << "10 Vectores estan vacios :" << "\n";
    }
}

void print_10_vectores_sa()
//imprime los 2 vectores como si estubiesemos haciendo un querry a la base de datos
{
    if (sa_ID.size() > 0)
    {    
    std::cout << "DebugPrint 10 Vectores sa_XXX :" << "\n";
        for (int i=0; i<sa_ID.size(); i++)
        {
            std::cout << sa_ID[i];
            std::cout << "|" << sa_IDREPS[i];
            std::cout << "|" << sa_RN1[i] << "|" << sa_RN2[i] << "|" << sa_RN3[i];
            std::cout << "|" << sa_PI1[i] << "|" << sa_PI2[i] << "|" << sa_PI3[i];
            std::cout << "|" << sa_PF1[i] << "|" << sa_PF2[i] << "|" << sa_PF3[i];
            std::cout << "\n";
        }
       
   
    }
    else
    {
        std::cout << "DebugPrint 10 Vectores :" << "\n";
        std::cout << "10 Vectores estan vacios :" << "\n";
    }
}

void print_10_vectores_cb()
//imprime los 2 vectores como si estubiesemos haciendo un querry a la base de datos
{
    if (cb_ID.size() > 0)
    {
        std::cout << "DebugPrint 10 Vectores cb_XXX :" << "\n";
        for (int i = 0; i < cb_ID.size(); i++)
        {
            std::cout << cb_ID[i];
            std::cout << "|" << cb_IDREPS[i];
            std::cout << "|" << cb_RN1[i] << "|" << cb_RN2[i] << "|" << cb_RN3[i];
            std::cout << "|" << cb_PI1[i] << "|" << cb_PI2[i] << "|" << cb_PI3[i];
            std::cout << "|" << cb_PF1[i] << "|" << cb_PF2[i] << "|" << cb_PF3[i];
            std::cout << "\n";
        }


    }
    else
    {
        std::cout << "DebugPrint 10 Vectores cb_XXX :" << "\n";
        std::cout << "10 Vectores estan vacios :" << "\n";
    }
}

void print_vector(std::vector<int> path)
{
    if (path.size() > 0)
    {
        std::cout << "DebugPrint Vector :" << "\n";
        for (auto i : path)
            std::cout << " -" << i << "- ";
        std::cout << "\n";
    }
}

//funciones para contar el tiempo que se tarda

void contador_comenzar(double total)
{
    contador_uno_porciento = total / 100;
    contador_hito = contador_uno_porciento;
    contador_porciento_actual = 0;
    std::cout << "Empezando proceso ";

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    std::cout << ss.str() << endl;

    return;
}

void contador_actualizar(double progreso) {
    if (progreso >= contador_hito) {
        contador_hito = contador_hito + contador_uno_porciento;
        contador_porciento_actual += 1;
        std::cout << contador_porciento_actual << "%" << '\r';
    }
    return;
}

void contador_terminar()
{
    //solo saca el tiempo now
    std::cout << "Proceso terminado ";

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    std::cout << ss.str() << endl;

    return;
}

//funciones que ya se usan

//mezclar vectores
//los vecotres temp_xx son los primero que se rellenan (cada sesion de juego)
//luego de cada sesion terminada, se vuelca el contenido de los veztores temp_xx a los vectores sa_xx
//luego antes de antes vuelca la base de datos los sa_xx , se hace el querry de las sa_ID en la base de datos
//se mezcla los vectores cb_XX (callback) con los vectores sa_XX, y ya se pasa a la base de datos los vectores sa_XX sobre escribiendo lo que hay

void borrar_vectores_sa()
{
    //BORRO VECTORES sa_XXX
    sa_ID.clear(); sa_IDREPS.clear();
    sa_RN1.clear(); sa_RN2.clear(); sa_RN3.clear();
    sa_PI1.clear(); sa_PI2.clear(); sa_PI3.clear();
    sa_PF1.clear(); sa_PF2.clear(); sa_PF3.clear();
}

void borrar_vectores_cb()
{
    //BORRO VECTORES cb_XXX
    cb_ID.clear(); cb_IDREPS.clear();
    cb_RN1.clear(); cb_RN2.clear(); cb_RN3.clear();
    cb_PI1.clear(); cb_PI2.clear(); cb_PI3.clear();
    cb_PF1.clear(); cb_PF2.clear(); cb_PF3.clear();
}

void borrar_vectores_temp()
{
    //BORRO VECTORES temp_XXX
    temp_ID.clear(); temp_IDREPS.clear();
    temp_RN1.clear(); temp_RN2.clear(); temp_RN3.clear();
    temp_PI1.clear(); temp_PI2.clear(); temp_PI3.clear();
    temp_PF1.clear(); temp_PF2.clear(); temp_PF3.clear();
}

void borrar_elementos_sobrantes_vectores_temp(int desde)
{
    //borra los elementos desde variable "desde" hasta el final
    temp_ID.erase(temp_ID.begin() + desde, temp_ID.end());
    temp_IDREPS.erase(temp_IDREPS.begin() + desde, temp_IDREPS.end());
    temp_RN1.erase(temp_RN1.begin() + desde, temp_RN1.end());
    temp_RN2.erase(temp_RN2.begin() + desde, temp_RN2.end());
    temp_RN3.erase(temp_RN3.begin() + desde, temp_RN3.end());
    temp_PI1.erase(temp_PI1.begin() + desde, temp_PI1.end());
    temp_PI2.erase(temp_PI2.begin() + desde, temp_PI2.end());
    temp_PI3.erase(temp_PI3.begin() + desde, temp_PI3.end());
    temp_PF1.erase(temp_PF1.begin() + desde, temp_PF1.end());
    temp_PF2.erase(temp_PF2.begin() + desde, temp_PF2.end());
    temp_PF3.erase(temp_PF3.begin() + desde, temp_PF3.end());

}

void mezclar_vectores_cb_y_sa() {
    //TRASBASA EL CONTENIDO DE LOS VECTORES cb_XXX (callback) A LOS VECTORES sa_XXX (los que tenemos en memoria)
    // 
    //mezcla los vectores cb_y_sa si los hay
    //primero vemos si se puede hacer por tamaños
    if (cb_ID.size() < 1) { std::cout << "No se puede mezclar, vector vacio, normalmente si la base de datos esta vacia" << endl; return; }
    if (cb_ID.size() > sa_ID.size()) { std::cout << "No se puede ser, vectores cb_xxx > sa_xxx" << endl; return; }

    //creo variable posicion
    int pos;

    for (int i = 0; i< sa_ID.size(); i++) {
        //busco si el ID_actual ya esta en el vector sa_ID
        vector<long long int>::iterator it = find(cb_ID.begin(), cb_ID.end(), sa_ID[i]);
        if (it != cb_ID.end()) {
            //cout << "Element " << sa_ID[i] << " found at position : ";
            //cout << it - cb_ID.begin() << endl;
            //si ID_actual SI esta presente sumo los vectores cb_xxx a sa_xxxx vectores
            pos = it - cb_ID.begin();
            // El sxx_ID[i] no se suma, por que es el key -------- sa_ID[i] = sa_ID[i] + cb_ID[pos];
            sa_IDREPS[i] = sa_IDREPS[i] + cb_IDREPS[pos];
            sa_RN1[i] = sa_RN1[i] + cb_RN1[pos]; sa_RN2[i] = sa_RN2[i] + cb_RN2[pos]; sa_RN3[i] = sa_RN3[i] + cb_RN3[pos];
            sa_PI1[i] = sa_PI1[i] + cb_PI1[pos]; sa_PI2[i] = sa_PI2[i] + cb_PI2[pos]; sa_PI3[i] = sa_PI3[i] + cb_PI3[pos];
            sa_PF1[i] = sa_PF1[i] + cb_PF1[pos]; sa_PF2[i] = sa_PF2[i] + cb_PF2[pos]; sa_PF3[i] = sa_PF3[i] + cb_PF3[pos];
            //print_10_vectores_sa();
        }
        else {
            //cout << "Element " << sa_ID[i] << " does not found" << endl;
            //si ID_actual no esta presente no hay que hacer nada
          
        }
        
    }

    //std::cout << "Vectores sumados 9 ultimas columnas que tienen misma xx_ID[]" << endl;
    //print_10_vectores_sa;
}

void mezclar_vectores_cb_y_temp() {
    //TRASBASA EL CONTENIDO DE LOS VECTORES cb_XXX (callback) A LOS VECTORES temp_XXX (los que tenemos en memoria)
    
    //mezcla los vectores cb_y_sa si los hay
    //primero vemos si se puede hacer por tamaños

    //hago un static flag para que no me de la matraca 1000 veces con que el vector esta vacio si la db es nueva
    static bool flag_vectores_vacios = false;
    
    if (cb_ID.size() < 1)
    {
        if (flag_vectores_vacios == false) 
        {
            flag_vectores_vacios = true;
            std::cout << "No se puede mezclar, vector cb vacio, normalmente si la base de datos esta vacia," << endl;
            std::cout << "o bien se estan pidiendo demasiadas Querrys y por eso no devuelve nada," << endl;
            std::cout << "o simplemente, no se ha encontrado ninguna coincidencia en la base de datos a lo pedido." << endl;
        }
        return; 
        
    }
    

    if (cb_ID.size() > temp_ID.size()) { std::cout << "No se puede ser, vectores cb_xxx > temp_xxx" << endl; return; }

    //creo variable posicion
    int pos;

    for (int i = 0; i < temp_ID.size(); i++) {
        //busco si el ID_actual ya esta en el vector temp_ID
        vector<long long int>::iterator it = find(cb_ID.begin(), cb_ID.end(), temp_ID[i]);
        if (it != cb_ID.end()) {
            //cout << "Element " << temp_ID[i] << " found at position : ";
            //cout << it - cb_ID.begin() << endl;
            //si ID_actual SI esta presente sumo los vectores cb_xxx a temp_xxxx vectores
            pos = it - cb_ID.begin();
            // El sxx_ID[i] no se suma, por que es el key -------- temp_ID[i] = temp_ID[i] + cb_ID[pos];
            temp_IDREPS[i] = temp_IDREPS[i] + cb_IDREPS[pos];
            temp_RN1[i] = temp_RN1[i] + cb_RN1[pos]; temp_RN2[i] = temp_RN2[i] + cb_RN2[pos]; temp_RN3[i] = temp_RN3[i] + cb_RN3[pos];
            temp_PI1[i] = temp_PI1[i] + cb_PI1[pos]; temp_PI2[i] = temp_PI2[i] + cb_PI2[pos]; temp_PI3[i] = temp_PI3[i] + cb_PI3[pos];
            temp_PF1[i] = temp_PF1[i] + cb_PF1[pos]; temp_PF2[i] = temp_PF2[i] + cb_PF2[pos]; temp_PF3[i] = temp_PF3[i] + cb_PF3[pos];
            //print_10_vectores_sa();
        }
        else {
            //cout << "Element " << temp_ID[i] << " does not found" << endl;
            //si ID_actual no esta presente no hay que hacer nada

        }

    }

    //std::cout << "Vectores sumados 9 ultimas columnas que tienen misma xx_ID[]" << endl;
    //print_10_vectores_sa;
}


void mezclar_vectores_temp_y_sa() {
    //EL TRABASE VA DE LOS VECTORES temp_XX (en memoria resultado del "dia") A LOS VECTORES sa_XX (en memoria acumulados se "dias")
    //mezcla los vectores temp_xx y _sa_xx (aunque este vacio)
    //este es un paso previo que se hace antes de meter en la base de datos,se consigue es eliminar duplicados antes de meter
    // a base de datos, esto es por que es mucho menos costoso hacerlo en memoria que mientras actualizando la base de datos
    // IMPORTANTE, itero los temp_xxx y si no esta en sa_XXX le hago pushback a sa_XXX, si si esta, sumo los int
    //primero vemos si se puede hacer por tamaños
    if (temp_ID.size() < 1) { std::cout << "los vectores temp_xx estan vacios, NO SE HA RECOGIDO DATOS" << endl; return; }
    
   
    //creo variable posicion
    int pos;

    //ES ALREVES BUSCO
    //NO SE QUE PASA SI EL TAMAÑO DE sa_xxx es 0
    for (int i = 0; i < temp_ID.size(); i++) {
        //busco si el ID_actual ya esta en el vector sa_ID
        vector<long long int>::iterator it = find(sa_ID.begin(), sa_ID.end(), temp_ID[i]);
        if (it != sa_ID.end()) {
            //cout << "Element " << sa_ID[i] << " found at position : ";
            //cout << it - temp_ID.begin() << endl;
            //si ID_actual SI esta presente sumo los vectores temp_xxx a sa_xxxx vectores
            pos = it - sa_ID.begin();
            // El sxx_ID[i] no se suma, por que es el key -------- sa_ID[i] = sa_ID[i] + temp_ID[pos];
            sa_IDREPS[pos] = sa_IDREPS[pos] + temp_IDREPS[i];
            sa_RN1[pos] = sa_RN1[pos] + temp_RN1[i]; sa_RN2[pos] = sa_RN2[pos] + temp_RN2[i]; sa_RN3[pos] = sa_RN3[pos] + temp_RN3[i];
            sa_PI1[pos] = sa_PI1[pos] + temp_PI1[i]; sa_PI2[pos] = sa_PI2[pos] + temp_PI2[i]; sa_PI3[pos] = sa_PI3[pos] + temp_PI3[i];
            sa_PF1[pos] = sa_PF1[pos] + temp_PF1[i]; sa_PF2[pos] = sa_PF2[pos] + temp_PF2[i]; sa_PF3[pos] = sa_PF3[pos] + temp_PF3[i];
            //print_10_vectores_sa();
        }
        else {
            //si no esta meto el vector el elemnto directamente en los vectores sa_XXX
            //cout << "Element " << sa_ID[i] << " does not found" << endl;
            //si ID_actual no esta presente en sa_XXX me toca agrandar el sa_XXX
            sa_ID.push_back(temp_ID[i]);
            sa_IDREPS.push_back(temp_IDREPS[i]);
            sa_RN1.push_back(temp_RN1[i]); sa_RN2.push_back(temp_RN2[i]); sa_RN3.push_back(temp_RN3[i]);
            sa_PI1.push_back(temp_PI1[i]); sa_PI2.push_back(temp_PI2[i]); sa_PI3.push_back(temp_PI3[i]);
            sa_PF1.push_back(temp_PF1[i]); sa_PF2.push_back(temp_PF2[i]); sa_PF3.push_back(temp_PF3[i]);


        }

    }

    //std::cout << "Vectores sumados 9 ultimas columnas que tienen misma xx_ID[]" << endl;
    //print_10_vectores_sa;
}


static int callback(void* data, int argc, char** argv, char** azColName) {   
    // Create a callback function 
    //SI LA DDBB DEVUELVE ALGO SE EJECUTA ESTA FUNCION QUE METE EN UN ARRAY LA REPUESTA/S

    std::string str;
    int numeroconvertido;
    int i;

    //borro contenido de cb_vector de callback previo
    //NO FUNCIONA POR QUE SI PIDO A LA BASE DE DATOS ALGO QUE NO TIENE, EL CODIGO NO LLEGA HASTA AQUI
    //EL CALLBACK NO SE EJECUTA SI LA BASE DE DATOS NO DEVUELVE NADA DESPES DEL QUERRY
    //std::cout << "Borro vector cb_vector, esto esta dentro del callback" << "\n";
    //cb_vector.clear();
    

    // es un callbak pero enfocado a como si hicieramos select en el querry
    
    //fprintf(stderr, "%s: ", (const char*)data);
    
    for (i = 0; i < argc; i++) {
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); //esta es la fila completa de la querry
        //pruebas para sacar solo el numero
        //std::cout << argv[i] <<"\n"; //este es el que saca la repuesta completa de la querry
        //cout << typeid(argv[i]).name() << endl;//TIPO QUE DEVOLVIA ES---- char * __ptr64

        
        //meterlo en vector
        str = argv[i];        
        numeroconvertido = std::atoi(str.c_str());
        //std::cout << "numero convertido es:" << numeroconvertido <<"fin" << "\n";
        cb_vector.push_back(numeroconvertido);
        
        
    }
    
    

    //printf("\n");
    return 0;
}

static int callback_masiva(void* data, int argc, char** argv, char** azColName) {
    // Create a callback function 
    //SI LA DDBB DEVUELVE ALGO SE EJECUTA ESTA FUNCION QUE METE EN UN ARRAY LA REPUESTA/S

    //Creo variables de respuesta para el bucle;

    //Strings
    std::string str;
   
    //numeros
    long long int num_lli;
    //int num_int;
    
    //bucle
    int i;
    int last_d; //last digit   

    //ATENCION si aqui borro los vectores de la DB,  solo recibiremos la fila 1 de lo que pidamos
    //borrar_vectores_cb();

    

    for (i = 0; i < argc; i++) {
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); //esta es la fila completa de la querry
        //pruebas para sacar solo el numero
        //std::cout << argv[i] <<"\n"; //este es el que saca la repuesta completa de la querry
        //cout << typeid(argv[i]).name() << endl;//TIPO QUE DEVOLVIA ES---- char * __ptr64

        //last_d = (i % 10); //en teoria esto saca el ultimo numero de un int, ESTO PUEDE QUE SOBRE
        last_d = i;
        str = argv[i];
        //meterlo en vector
        switch (last_d) {
            //todos son convertidos a long long int (atoll)
            case 0:  num_lli = std::atoll(str.c_str()); cb_ID.push_back(num_lli); break;
            case 1:  num_lli = std::atoll(str.c_str()); cb_IDREPS.push_back(num_lli); break;
            case 2:   num_lli = std::atoll(str.c_str()); cb_RN1.push_back(num_lli); break;
            case 3:   num_lli = std::atoll(str.c_str()); cb_RN2.push_back(num_lli); break;
            case 4:   num_lli = std::atoll(str.c_str()); cb_RN3.push_back(num_lli); break;
            case 5:   num_lli = std::atoll(str.c_str()); cb_PI1.push_back(num_lli); break;
            case 6:   num_lli = std::atoll(str.c_str()); cb_PI2.push_back(num_lli); break;
            case 7:   num_lli = std::atoll(str.c_str()); cb_PI3.push_back(num_lli); break;
            case 8:   num_lli = std::atoll(str.c_str()); cb_PF1.push_back(num_lli); break;
            case 9:   num_lli = std::atoll(str.c_str()); cb_PF2.push_back(num_lli); break;
            case 10:   num_lli = std::atoll(str.c_str()); cb_PF3.push_back(num_lli); break;
        }

    }
    //printf("\n");
    return 0;
}

void db_crear_borrar(const char* db_nombre) {
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

int db_crear(const char* database_name)
{
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
        rc = sqlite3_open(database_name, &db);

        if (rc) {
            // Show an error message
            std::cout << "DB Error: " << sqlite3_errmsg(db) << std::endl;
            // Close the connection
            sqlite3_close(db);
            // Return an error
            return(1);
        }
  

        //Esta la modifique para creat la base de datos
        sql = "CREATE TABLE SA ( ID INTEGER PRIMARY KEY NOT NULL,IDREPS INTEGER NOT NULL, RN1 INTEGER NOT NULL,RN2 INTEGER NOT NULL, RN3 INTEGER NOT NULL,PI1 INTEGER NOT NULL,PI2 INTEGER NOT NULL, PI3 INTEGER NOT NULL,PF1 INTEGER NOT NULL,PF2 INTEGER NOT NULL, PF3 INTEGER NOT NULL);";

        //Esta es para debug en solo 1 fila
        //Esta la modifique para creat la base de datos
        //sql = "CREATE TABLE SA ( ID INTEGER PRIMARY KEY NOT NULL, RN1 INTEGER NOT NULL);";


        /* Open database, a ver si abriendola no da problemas PUEDE QUE SOBRE por estar ya creada */
        //rc = sqlite3_open("sa.db", &db);

        // Run the SQL (convert the string to a C-String with c_str() )
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

        //set journal mode to memory, es para que no cree y borre filas temporales con acceso a disco
        sql = "PRAGMA journal_mode=MEMORY;";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

        //meterdatos
        /*
        sql = "REPLACE INTO SA VALUES (100200300,1,2,3,4,5,6,7,8,99);";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        */

        //veo meto un indice con numero tocho
        /*
        std::cout << "meto indice tocho" << "\n";
        long long int dbl01 = 1010101020202030303;
        int n9 = 69;
        sql = "REPLACE INTO SA VALUES (" + std::to_string( dbl01) + ", 133, 233, 333,4,5,6,7,8," + std::to_string(n9) +"); ";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        print_vector(cb_vector);
        std::cout << "fin meto indice tocho" << "\n";
        */
        
       

       
        /*
        //borro un posible vector creado en el callback
        cb_vector.clear();
        // Create SQL statement
        std::cout << "SELECT RN1,RN2,RN3,PI1,PI2,PI3,PF1,PF2,PF3  FROM SA WHERE rowid IN" << "\n";
        sql ="SELECT RN1,RN2,RN3,PI1,PI2,PI3,PF1,PF2,PF3  FROM SA WHERE rowid IN(100200300);"; 
        //Execute SQL statement
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);
        */
       

        // Close the SQL connection
        sqlite3_close(db);

        return 0;
    
}

int db_update(const char* database_name)
{
    //Creo variaqbles necesarios para albergar datos del updata
    long long int db_ID;
    int db_RN1; int db_RN2; int db_RN3; int db_PI1; int db_PI2; int db_PI3; int db_PF1; int db_PF2; int db_PF3;


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
    rc = sqlite3_open(database_name, &db);

    if (rc) {
        // Show an error message
        std::cout << "DB Error: " << sqlite3_errmsg(db) << std::endl;
        // Close the connection
        //sqlite3_close(db);
        // Return an error
        return(1);
    }

   
    
    /*
    //meterdatos EJEMPLO
    sql = "REPLACE INTO SA VALUES (100200300,1,2,3,4,5,6,7,8,9);";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //veo meto un indice con numero tocho EJEMPLO
    long long int dbl01 = 1010101020202030303;
    sql = "REPLACE INTO SA VALUES (" + std::to_string(dbl01) + ", 133, 233, 333,4,5,6,7,8,99); ";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    */

    //trato de evitar que no cree los temporales -journal
    sql = "PRAGMA journal_mode=MEMORY;";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //CREO UNA LLAMADAQ MUY GORDA A SQL QUE LA VOY LUEGO A PASAR LOS VECTORES
    //TENGO QUE CAMBIAR EL CALL BACK



    //DE AQUI PARA ABAJO BORRAR
    //BUCLE EN EL QUE METO TODA LA MATRIZ (FORMADA POR 10 VECTORES sa_xxx) en da BBDD
    for (int j = 0; j < sa_ID.size(); j++)
    {
        
        //Creo/obtengo el sa_ID a aplicar
        //db_ID = 700200300; //DEBUG
        
        db_ID = sa_ID[j];
        db_RN1 = sa_RN1[j]; db_RN2 = sa_RN2[j]; db_RN3 = sa_RN3[j];
        db_PI1 = sa_PI1[j]; db_PI2 = sa_PI2[j]; db_PI3 = sa_PI3[j];
        db_PF1 = sa_PF1[j]; db_PF2 = sa_PF2[j]; db_PF3 = sa_PF3[j];

        

        //LEO LA FILA REQUERIDA,menos el ID
        // Create SQL statement

        
        
        sql = "SELECT RN1,RN2,RN3,PI1,PI2,PI3,PF1,PF2,PF3  FROM SA WHERE rowid IN(" + std::to_string(db_ID) + "); ";
        // Execute SQL statement 
        cb_vector.clear();
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);
        
        //std::cout << "SELECT RN1,RN2,RN3,PI1,PI2,PI3,PF1,PF2,PF3  FROM SA WHERE rowid IN(" + std::to_string(db_ID) + ")" << "\n";
        //std::cout << "LA QUERRY DEVOLVIO ESTE VECTOR DE LA DATA BASE" <<  "\n";
        //print_vector(cb_vector);
        //std::cout << "cb_vector tamano=" << cb_vector.size() << "\n";
        //std::cout << "\n";

        //SUMO LOS RESULTADOS DE LA FILA OBTENIDO EN cb_vector, QUE ES LO QUE DEVUELVE LA BASE DE DATOS SI EXISTE EL ROW
        //  a las que saque de los vectores creados sa_RN1, sa_RN2,....,sa_PF3
        //TODO CAMBIAR O POR i    

        //SI EL CALLBACK NOS DEVUELVE VECTOR HAY QUE SUMAR EL INT QUE YA ESTABA EN LA BASE DE DATOS
        if (cb_vector.size() > 0)
        {
            db_RN1 = db_RN1 + cb_vector[0]; db_RN2 = db_RN2 + cb_vector[1]; db_RN3 = db_RN3 + cb_vector[2];
            db_PI1 = db_PI1 + cb_vector[3]; db_PI2 = db_PI2 + cb_vector[4]; db_PI3 = db_PI3 + cb_vector[5];
            db_PF1 = db_PF1 + cb_vector[6]; db_PF2 = db_PF2 + cb_vector[7]; db_PF3 = db_PF3 + cb_vector[8];
        }



        //METER LOS RESULTADOS A LA BASE DE DATOS   
        //sql = "REPLACE INTO SA VALUES (" + std::to_string(dbl01) + ", 133, 233, 333,4,5,6,7,8,9); ";
        sql = "REPLACE INTO SA VALUES (" + std::to_string(db_ID) + "," + std::to_string(db_RN1) + ", " + std::to_string(db_RN2) + ", " + std::to_string(db_RN3) + "," + std::to_string(db_PI1) + "," + std::to_string(db_PI2) + "," + std::to_string(db_PI3) + "," + std::to_string(db_PF1) + "," + std::to_string(db_PF2) + "," + std::to_string(db_PF3) + "); ";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        
        
        

        /*
        //test con leyendo solo una columna
        //borrar de aqui a corchete
        // 
        //LEO LA FILA REQUERIDA,menos el ID        
        sql = "SELECT RN1 FROM SA WHERE rowid IN(" + std::to_string(db_ID) + "); ";
        // Execute SQL statement
        cb_vector.clear();
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);


        sql = "REPLACE INTO SA VALUES (" + std::to_string(db_ID) + ",1994020994020992020); ";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        */
    }

    /* Create SQL statement */
    //std::cout << "Tercera llamada que pido SELECT * RN2 FROM SA WHERE ID=" << "\n";        
    //sql = "SELECT  RN2 FROM SA WHERE ID=" + std::to_string(dbl01) + ";";

    /* Execute SQL statement */
    //rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);

    //DEBUG PRINT
    //std::cout << "Este es el id sacado " << std::to_string(resp01) << "\n";

    // Close the SQL connection
    sqlite3_close(db);
    
    return 0;

}

int db_update_masivo(const char* database_name)
{
    //Salgo si el vector sa_ID está vacio
    //tamaño del vector de las ID creado, se usa mas adelante en bucles
    int tam = temp_ID.size();
    //salgo si el vector está vacio
    if (tam == 0) { return 1; }

    //SE PUEDE BORRAR POR QUE EL CALLBACK VA EN VECTORES GEMELOS
    //Creo variaqbles necesarios para albergar datos del updata
    //long long int db_ID;
    //int db_RN1; int db_RN2; int db_RN3; int db_PI1; int db_PI2; int db_PI3; int db_PF1; int db_PF2; int db_PF3;


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
    rc = sqlite3_open(database_name, &db);

    if (rc) {
        // Show an error message
        std::cout << "DB Error: " << sqlite3_errmsg(db) << std::endl;
        // Close the connection
        //sqlite3_close(db);
        // Return an error
        return(1);
    }



    /*
    //meterdatos EJEMPLO
    sql = "REPLACE INTO SA VALUES (100200300,1,2,3,4,5,6,7,8,9);";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //veo meto un indice con numero tocho EJEMPLO
    long long int dbl01 = 1010101020202030303;
    sql = "REPLACE INTO SA VALUES (" + std::to_string(dbl01) + ", 133, 233, 333,4,5,6,7,8,99); ";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    */

    //trato de evitar que no cree los temporales -journal
    sql = "PRAGMA journal_mode=MEMORY;";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //CREO UNA LLAMADAQ MUY GORDA A SQL QUE LA VOY LUEGO A PASAR LOS VECTORES
    //TENGO QUE CAMBIAR EL CALL BACK

    

    
    
    //BUCLE EN EL LEO TODOS LAS FILAS QUE QUE TIENEN UN ID QUE ESTE EN el vector de en memorya temp_ID
    //El callback_masivo me va a rellenar ya los vectores cb_ID 
    // Create SQL statement
    sql = "SELECT ID,IDREPS,RN1,RN2,RN3,PI1,PI2,PI3,PF1,PF2,PF3 FROM SA WHERE ";
    for (int j = 0; j < tam; j++)
    {     
        //Sigo creando el SQL statement        
        sql=sql + "ID=" + std::to_string(temp_ID[j]) ;
        //veo si es el ultimo ID, que termino o no que pongo un OR
        if (j < (tam - 1)) {  sql = sql + " OR ";} else {  sql = sql + " ;";}    
        //std::cout << sql << endl;
    }
    //querry terminada al hacer terminar el bucle
    //std::cout << sql << endl;

    //BORRO EL LOS VECTORES cb (de callback)
    borrar_vectores_cb();    

    // Execute SQL statement         
    rc = sqlite3_exec(db, sql.c_str(), callback_masiva, (void*)data, &zErrMsg);
    //DEBUG imprimo los vectores que nos da
    //print_10_vectores_cb();    

    //BUCLE PARA ACTUALIZAR LOS VECTORES temp_XXX que es un slice de temp_xxx con lo que hemos sacado de la db
    mezclar_vectores_cb_y_temp();

    //BORRO vectores_cb para que no ocupen memoria al meter los datos a la data base
    borrar_vectores_cb();

    //BLUCLE PARA ACTUALIZAR LA DB CON LOS VECTORES TEMP
    // Create SQL statement
    sql = "REPLACE INTO SA (ID,IDREPS,RN1,RN2,RN3,PI1,PI2,PI3,PF1,PF2,PF3 ) VALUES ";
    for (int h = 0; h < temp_ID.size(); h++)
    {
        //Sigo creando el SQL statement        
        sql = sql + "(" + std::to_string(temp_ID[h]);
        sql = sql + "," + std::to_string(temp_IDREPS[h]);
        sql = sql + "," + std::to_string(temp_RN1[h]) + "," + std::to_string(temp_RN2[h]) + "," + std::to_string(temp_RN3[h]);
        sql = sql + "," + std::to_string(temp_PI1[h]) + "," + std::to_string(temp_PI2[h]) + "," + std::to_string(temp_PI3[h]);
        sql = sql + "," + std::to_string(temp_PF1[h]) + "," + std::to_string(temp_PF2[h]) + "," + std::to_string(temp_PF3[h]);
        sql = sql + ")";

        //veo si es el ultimo ID, que termino o no que pongo un OR
        if (h < (tam - 1)) { sql = sql + ","; }
        else { sql = sql + ";"; }
    }
    //querry terminada al hacer terminar el bucle
    //std::cout << sql << endl;
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    //SUMO LOS RESULTADOS DE LA FILA OBTENIDO EN cb_vector, QUE ES LO QUE DEVUELVE LA BASE DE DATOS SI EXISTE EL ROW
    //  a las que saque de los vectores creados sa_RN1, sa_RN2,....,sa_PF3
    //TODO CAMBIAR O POR i    

    //SI EL CALLBACK NOS DEVUELVE VECTOR HAY QUE SUMAR EL INT QUE YA ESTABA EN LA BASE DE DATOS  


    //AQUI BUCLE PARA METER LOS DATOS
    //METER LOS RESULTADOS A LA BASE DE DATOS   
    //sql = "REPLACE INTO SA VALUES (" + std::to_string(dbl01) + ", 133, 233, 333,4,5,6,7,8,9); ";
    //sql = "REPLACE INTO SA VALUES (" + std::to_string(db_ID) + "," + std::to_string(db_RN1) + ", " + std::to_string(db_RN2) + ", " + std::to_string(db_RN3) + "," + std::to_string(db_PI1) + "," + std::to_string(db_PI2) + "," + std::to_string(db_PI3) + "," + std::to_string(db_PF1) + "," + std::to_string(db_PF2) + "," + std::to_string(db_PF3) + "); ";
    //rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);


    

    // Close the SQL connection
    sqlite3_close(db);

    return 0;

}

long long int crear_ID_actual(long long int lli_dado, std::vector<int> v_dado)
{
    //Crea una long long int a partir de un long long int y un vector con los incrementos en sus digitos
    //lli_dado es 1 r1 r2 r3 p1 p2 p3 f1 f2 f3, siempre 1 delante y luego digitos en pares que corresponden a las reps 
    //v_dado tiene que tener 9 posiciones v_dado.size()=9

    /*
    //override debug, COMENTAR ESTE BLOQUE PARA QUE FUNCIONE LA FUNCION
    lli_dado = 1010203040506070809;
    v_dado.clear();
    v_dado = { 1,2,3,4,5,6,7,8,9 };
    */

    //convierto lli_dado a str
    std::string str_lli_dado = std::to_string(lli_dado);
    long long int lli_devuelto = lli_dado;  std::string str_lli_devuelto = str_lli_dado;
    int int_extracto; std::string str_extracto;
    int int_inserto; std::string str_inserto;
    int ini_pos = 1;
    for (int h = 0; h < 9; h++) {
        if (v_dado[h] > 0) {
            //saco numero de la cadena del long long int y lo sumo al del vector
            std::istringstream(str_lli_dado.substr(ini_pos, 2)) >> int_extracto;
            int_inserto = int_extracto + v_dado[h];

            //WARNING SI ES MAS 2 99 SERIAN 3 CARACTERES Y FASTIDIARIA EL SISTEMA DE NUMERACION, PARO CON UN STD::CIN
            //ESTE ERROR SE DA CUANDO EN SESIONES DIARIAS DE + DE 99 BOLAS SE PRODUCEN +99 REPS DE ALGO
            if (int_inserto > 99) { std::cout << "ERROR NUMERO >99"; std::cin >> int_inserto; }

            //paso la suma a str de 2 caracteres
            str_inserto = std::to_string(int_inserto);
            if (str_inserto.size() < 2) { str_inserto = "0" + str_inserto; }

            //meto inserto en su posicion sobreescribiendo lo que esta en las posiciones que va el insero
            str_lli_devuelto.replace(ini_pos, 2, str_inserto);
        }
        ini_pos = ini_pos + 2;
    }
    //cambio cadena a long long int
    lli_devuelto = std::stoll(str_lli_devuelto);
    //std::cout << "Cadena1 " << lli_dado << " y Cadena2 " << lli_devuelto << "\n";

    return lli_devuelto;
}
long long int crear_ID_actual_V2(long long int& lli_dado, std::vector<int>& v_dado, std::vector<int>& last_reps)
{
    //Crea una long long int a partir de un long long int y un vector con los incrementos en sus digitos
    //lli_dado es 1|bola_n|r1|r2|r3|p1|p2|p3|f1|f2|f3, siempre 1 delante y luego digitos en pares que corresponden a las reps 
    //v_dado tiene que tener 9 posiciones v_dado.size()=9
    // la pos 0 es la bola_n, que siempre será una bola mas que la anterior
    //las posiciones 0,1,3,4,6,7 son siempre de 2 digitos, que serian las reps de 1 y de 2
    //las posiciones 2,5,8 son de 1 digito son los de 3 o mas reps
    //esto es asi por que en un long long int solo me dejan 19 digitos y si meto la tercera rep en 2 digitos me meto en 21 digitos


    /*
    //override debug, COMENTAR ESTE BLOQUE PARA QUE FUNCIONE LA FUNCION
    long long int lli_dado;
    std::vector<int> v_dado;
    v_dado.clear();
    lli_dado = 104010230102301023;    
    v_dado = { 20,10,5,20,10,5,20,10,5 };
    */
    

    // cambio posiciones 20220801 son 1 bb (bolas salidas) c(ultima rep salida) dd(reps1) d(reps2) d (reps3)
    // quedaria 1bbcddddef  f  f  f  g  h  h h   h
    //          0123456789 10 11 12 13 14 15 16 17
    // entonces posicio numero de bolas salidas = 1-2
    // ultima rep RN = 3, repsRN1=4-5, repsRN2=6,repsRN3=7
    // ultima rep PI = 8, repsPI1=9-10, repsPI2=11,repsPI3=12
    // ultima rep PF = 13, repsPF1=14-15, repsPF2=16,repsPF3=17

    //creo un vector de 16 posiciones que son las que voy a cambiar en el nuevo ID
    std::vector<int> vdc; //Vector de Cambios
    //copio v_dado
    for (int i = 0; i < v_dado.size(); i++) {
        vdc.push_back(v_dado[i]);
    }
    //inserto las posciones de de vector last reps en los sitios que corresponden
    vdc.insert(vdc.begin(), last_reps[0]);
    vdc.insert(vdc.begin() + 4, last_reps[1]);
    vdc.insert(vdc.begin() + 8, last_reps[2]);
    //fin creacion vector vdc //vector de cambios


    //print_vector(vdc);

    //convierto lli_dado a str
    std::string str_lli_dado = std::to_string(lli_dado);
    long long int lli_devuelto = lli_dado;  std::string str_lli_devuelto = str_lli_dado;
    int int_extracto; std::string str_extracto;
    int int_inserto; std::string str_inserto;
    int ini_pos = 1;

    //bolas salidas =sumar a la posicion 1 y 2 un +1 y convertirlo a string y lli
    // 
    // primer numero sumo
    // 
    //saco numero de la cadena del long long int posiciones 1-2 y lo sumo al del vector
    std::istringstream(str_lli_dado.substr(ini_pos, 2)) >> int_extracto;
    int_inserto = int_extracto + 1;
    if (int_inserto > 99) { std::cout << "ERROR NUMERO QUE TRATAMOS>99"; std::cin >> int_inserto; }
    str_inserto = std::to_string(int_inserto);
    if (str_inserto.size() < 2) { str_inserto = "0" + str_inserto; }
    //meto inserto en su posicion sobreescribiendo lo que esta en las posiciones que va el insero
    str_lli_devuelto.replace(ini_pos, 2, str_inserto); 

    
    
    ini_pos = 3;

    //bucle para insertar las posiciones de vdc vector de cambios
    for (int h = 0; h < vdc.size(); h++) {
        if (vdc[h] >= 0) {
            switch (h)
                {
                // forma de proceder para los casos de un solo digito que no se tienen que sumar, los last_reps
                // que contienen el ultimo grupo de reps que salió
                case 0:
                case 4:
                case 8:
                    str_inserto = std::to_string(vdc[h]);
                    str_lli_devuelto.replace(ini_pos, 1, str_inserto);
                    break;
                //forma de proceder para casos solo 1 digit0, que se tiene que sumar
                case 2:
                case 3:                
                case 6:
                case 7:                
                case 10:
                case 11:
                    //TODO ver como hacer con 1 solo digito
                     //saco numero de la cadena del long long int y lo sumo al del vector
                    std::istringstream(str_lli_dado.substr(ini_pos, 1)) >> int_extracto;
                    int_inserto = int_extracto + vdc[h];

                    //WARNING SI ES MAS 2 99 SERIAN 3 CARACTERES Y FASTIDIARIA EL SISTEMA DE NUMERACION, PARO CON UN STD::CIN
                    //ESTE ERROR SE DA CUANDO EN SESIONES DIARIAS DE + DE 99 BOLAS SE PRODUCEN +99 REPS DE ALGO
                    if (int_inserto > 9) { std::cout << "ERROR NUMERO QUE TRATAMOS>9"; std::cin >> int_inserto; }

                    //paso la suma a str de 2 caracteres
                    str_inserto = std::to_string(int_inserto);
                    //if (str_inserto.size() < 2) { str_inserto = "0" + str_inserto; }

                    //meto inserto en su posicion sobreescribiendo lo que esta en las posiciones que va el insero
                    str_lli_devuelto.replace(ini_pos, 1, str_inserto);

                    
                    
                    break;

                //forma de proceder con los casos de 2 digitos
                default:
                    //saco numero de la cadena del long long int y lo sumo al del vector
                    std::istringstream(str_lli_dado.substr(ini_pos, 2)) >> int_extracto;
                    int_inserto = int_extracto + vdc[h];

                    //WARNING SI ES MAS 2 99 SERIAN 3 CARACTERES Y FASTIDIARIA EL SISTEMA DE NUMERACION, PARO CON UN STD::CIN
                    //ESTE ERROR SE DA CUANDO EN SESIONES DIARIAS DE + DE 99 BOLAS SE PRODUCEN +99 REPS DE ALGO
                    if (int_inserto > 99) { std::cout << "ERROR NUMERO QUE TRATAMOS>99"; std::cin >> int_inserto; }

                    //paso la suma a str de 2 caracteres
                    str_inserto = std::to_string(int_inserto);
                    if (str_inserto.size() < 2) { str_inserto = "0" + str_inserto; }

                    //meto inserto en su posicion sobreescribiendo lo que esta en las posiciones que va el insero
                    str_lli_devuelto.replace(ini_pos, 2, str_inserto);

                   
                    break;
                }
               
        }        
        switch (h)
        {
            //forma de proceder para casos solo 1 digit0
        case 0:
        case 2:
        case 3:
        case 4:
        case 6:        
        case 7:
        case 8:
        case 10:
        case 11:            
            ini_pos = ini_pos + 1;
            break;
        default:
            ini_pos = ini_pos + 2;
            break;
        }
       
    }

    //cambio cadena texto creada en la funcion a long long int que es lo que tenemos que devolver
    lli_devuelto = std::stoll(str_lli_devuelto);
    //std::cout << "Cadena1 " << lli_dado << endl;
    //std::cout << "Cadena2 " << lli_devuelto << "\n";

    return lli_devuelto;
}

int lanzar_bola()
{
    //se lanza una bola random entre 1 y 36
    //Genera numero true random
    // https://www.bogotobogo.com/cplusplus/RandomNumbers.php
    //srand((int)time(0));      //  TRUE RANDOM queda comentado y se pone al principio de cada sesion diaria, para que no coma procesador
    int bola = (rand() % 36) + 1;
    //cout << bola << " ";
    
    
    
    //se mete en los arrays historicos, como 0 o 1 (rojo0negro1 par0impar1 pasa0falta1)
    //rojo0negro1
    int rn;
    switch (bola) {
        case 1:rn = 0; break;
        case 2:rn = 1; break;
        case 3:rn = 0; break;
        case 4:rn = 1; break;
        case 5:rn = 0; break;
        case 6:rn = 1; break;
        case 7:rn = 0; break;
        case 8:rn = 1; break;
        case 9:rn = 0; break;
        case 10:rn = 1; break;
        case 11:rn = 1; break;
        case 12:rn = 0; break;
        case 13:rn = 1; break;
        case 14:rn = 0; break;
        case 15:rn = 1; break;
        case 16:rn = 0; break;
        case 17:rn = 1; break;
        case 18:rn = 0; break;
        case 19:rn = 0; break;
        case 20:rn = 1; break;
        case 21:rn = 0; break;
        case 22:rn = 1; break;
        case 23:rn = 0; break;
        case 24:rn = 1; break;
        case 25:rn = 0; break;
        case 26:rn = 1; break;
        case 27:rn = 0; break;
        case 28:rn = 1; break;
        case 29:rn = 1; break;
        case 30:rn = 0; break;
        case 31:rn = 1; break;
        case 32:rn = 0; break;
        case 33:rn = 1; break;
        case 34:rn = 0; break;
        case 35:rn = 1; break;
        case 36:rn = 0; break;
    }
    hist_RN.push_back(rn);

    //par0impar1
    if (bola % 2 == 0) {hist_PI.push_back(0);} else {hist_PI.push_back(1);}

    //pasa0falta1
    if (bola >= 19) {
        hist_PF.push_back(0);
    }
    else {
        hist_PF.push_back(1);
    }

    return bola;
}

int actualizar_vectores_historicos(std::vector<int>& v_h) {
    //Ver si se ha producido una repeticion confirmada de Ceros o Unos, si es asi devuelvo se se repiteron 1,2,3+
    //Actualizacion 2220725
    //Devuelve 0 si el vector esta vacio, o si no se ha empezado a formar grupo de reps desde la primerabola obtenida
    //devuelve un int del 1 al 3 si detecta un grupo de repeticiones de 1 a 3 o mas ( si repite >devuelve 3 igualemente)
    //devuelve un numero negativo si un bloque de xx repeticiones se esta repitiendo pero no ha terminada
    //en resumen excepto si el vector esta vacio o es la primera acumulacion sin definir {0,0,0,0} el vector nunca devolvera 0
    //despues de la primera repeticion fructifera ejemplo  {0,0,0,1,1,0} devuelve 2 (las perpeticiones de 1,1 y luego
    // deja el vector en {1,0} que es los ultimos 2 digitos que marcaran ya que la siguiente ronda de reps empieza con 0
    //Actualizacion 20220726
    //ahora devuelve 3 en cuanto se alcanza las 3 repeticiones (antes devolvia -3) y recorta el vector
    //esto implica que ahora al al salir 4 o mas veces repetido algo retornará 0 por que se recorta el vector
    //ejemplo {1,0,0,0,0,0,0} retorna max_reps (3) y deja vector {0,0,0}
    
    
    int start_element=0;
    int reps=0;
    int tamano = v_h.size();
    int max_reps = 3; //esto es un SETTING, cualquier reps > max_reps se cuenta como si fueran rmax_reps

    if (tamano == 0) { return 0; }

    //veo si el vector tiene las condiciones para encontrar una repeticion
    //debe tener al menos  2 elementos y que los 
    if (tamano > 2) // && v_h[tamano - 1] != v_h[tamano - 2]) //solo será =<1 en la primera tirada
    {
        for (int i = 1; i < tamano; i++)
        {
            if (v_h[i] != v_h[i - 1])
            {
                start_element = i;
                break;
            }
        }
    }
    else
    {
        switch (tamano) {
        case 1: return 0; break;
        case 2: if (v_h[tamano - 1] != v_h[tamano - 2]) { return -1; break; }
        default: return 0; break;
        }

    }

    
    
    //necesto que ademas el primer elemento a inspeccionar se diferente del penultimo
    //ejemplo 0,0,0,0,1  no sirve, pero 0,0,1(seria el start_element,1,1,0 si 
    if (start_element != 0) // && start_element != (tamano-1))
    {
        //ya se que tiene una rep
        reps = 1;
        //busco si tiene mas reps
        for (int j = start_element; j < tamano-1; j++)
        {
            if (v_h[j] == v_h[j + 1])
            {
                reps = reps + 1;
            }
            else 
            {
                break;
            }
        }
        
    }
  

    

    //vemos si la cadena está terminada con el ultimo elemento diferente al penultimo
    if (v_h[tamano - 1] == v_h[tamano - 2] || start_element == tamano-1)
    {
        reps = -1 * reps;
        //return reps;
    }

    if (abs(reps) >= max_reps) { reps = max_reps; }
    

    //si sale al menos una repeticion borro la parte del vector ya vista, osea solo dejo los 2 ultimos digitos
    if (reps > 0) 
    {
        //Ej:Tenia el vector 0,1,1,1,0, ya voy a devolver la repeticion de 3, por lo que dejo el vector en 1,0 (las 2 ultimas pos)
        // Osea que tengo que borrar todas las posicones del vector menos las 2 últimas
        //vector.erase(vector.begin() + 3, vector.begin() + 5); // Deleting from fourth element to sixth element
        v_h.erase(v_h.begin() +0, v_h.begin() + start_element-1 + reps); // Deleting from fourth element to sixth element
        
    }    
    

    //si las repeticones son mayores que 3 se quedan en 3
    if (reps > max_reps) { reps = max_reps; }

    

    return reps;
}

void actualizar_vectores_temp_XXX(short int repsRN,short int repsPI,short int repsPF) {
    //antes era actualizar_vectores_sa_XXX por eso el codigo comentado puede estar mal si se quiere resucitar
    

    

    //busco si se ha encontrado alguna rep y si es asi actualizo los vectores en el momento pasado que se origino la rep

    int rn1=0; int rn2=0; int rn3=0; int pi1=0; int pi2=0; int pi3=0; int pf1=0; int pf2=0; int pf3=0;
    
    //cambiamos el que no sea 0 por el/los que cambiasen
    switch (repsRN) { case 1: rn1 = 1; break; case 2: rn2 = 1; break; case 3:  rn3 = 1; break; }
    switch (repsPI) { case 1: pi1 = 1; break; case 2: pi2 = 1; break; case 3:  pi3 = 1; break; }
    switch (repsPF) { case 1: pf1 = 1; break; case 2: pf2 = 1; break; case 3:  pf3 = 1; break; }

    //Dejo preparada la nueva id para que cuando se llame esta funcion de nuevo se cree el nuevo vector
    //Calculo el ID_nuevo a partir del id actual y los datos ye que tengo aqui en variables, si bien lo pongo en los vectores
    std::vector<int> ristra_reps =  { rn1,rn2,rn3,pi1,pi2,pi3,pf1,pf2,pf3 };
    //meto cual es la ultima repeticion que se ha producido tb, para incorporarlo a la ID
    std::vector<int> last_reps = { repsRN,repsPI,repsPF };
    ID_nueva = crear_ID_actual_V2(ID_actual, ristra_reps, last_reps); //el vector ya se creara vacio en la siguiente vuelta

    //pongo a la id_actual EN temp_ID y todo ceros creando el vector creando el vector del id actual
    temp_ID.push_back(ID_actual);
    temp_IDREPS.push_back(1); //siempre se añade solo 1 por que solo sale una vez esa id por dia
    temp_RN1.push_back(0); temp_RN2.push_back(0); temp_RN3.push_back(0);
    temp_PI1.push_back(0); temp_PI2.push_back(0); temp_PI3.push_back(0);
    temp_PF1.push_back(0); temp_PF2.push_back(0); temp_PF3.push_back(0);
    //dejo puesta la id nueva como actual, solo servirá para la proxima vex que se llame a esta funcion
    ID_actual = ID_nueva;

    
    //aqui tengo que poner la manera de volver al pasado en los vectores temp_XXX y poner cuando se iniciaron las reps
    int pos;//es la poscion del vector(pos  0=1)
    //vel cual es la ultima poscion del vector (la primera es 0)
    pos = temp_ID.size() - 1;
    //salgo de la funcion si no hay al menos 2 elementos en el vector
    if (pos <= 0) { return; }
    
    

    //relleno las posiciones en el pasado segun las repeticiones detectadas

    if (rn1 == 1) { temp_RN1[pos - 1] = 1;}
    if (rn2 == 1) { temp_RN2[pos - 2] = 1; }
    if (rn3 == 1) { temp_RN3[pos - 2] = 1; } //MUY IMPORTANTE deberia ser pos-3, pero la funcion actualizar_vectores_historicos nos devuelve 3 nada mas que se consigue el 3
    if (pi1 == 1) { temp_PI1[pos - 1] = 1; }
    if (pi2 == 1) { temp_PI2[pos - 2] = 1; }
    if (pi3 == 1) { temp_PI3[pos - 2] = 1; }
    if (pf1 == 1) { temp_PF1[pos - 1] = 1; }
    if (pf2 == 1) { temp_PF2[pos - 2] = 1; }
    if (pf3 == 1) { temp_PF3[pos - 2] = 1; }

    //print_10_vectores_sa();

    /*
    //busco si el ID_actual ya esta en el vector sa_ID
    vector<long long int>::iterator it = find(sa_ID.begin(), sa_ID.end(), ID_actual);
    if (it != sa_ID.end()) {
        //cout << "Element " << ID_actual << " found at position : ";
        //cout << it - sa_ID.begin() << " al actualizar el vector generado en memoria" << endl;
        //si ID_actual SI esta presente reescribo la posicion en vectores
        pos = it - sa_ID.begin();
        sa_ID[pos]= ID_actual;
        sa_RN1[pos] = sa_RN1[pos]+rn1; sa_RN2[pos] = sa_RN2[pos] + rn2; sa_RN3[pos] = sa_RN3[pos] + rn3;
        sa_PI1[pos] = sa_PI1[pos] + pi1; sa_PI2[pos] = sa_PI2[pos] + pi2; sa_PI3[pos] = sa_PI3[pos] + pi3;
        sa_PF1[pos] = sa_PF1[pos] + pf1; sa_PF2[pos] = sa_PF2[pos] + pf2; sa_PF3[pos] = sa_PF3[pos] + pf3;
    }
    else {
        //cout << "Element " << ID_actual << " does not found al actualizar el vector generado en memoria" << endl;
        //si ID_actual no esta presente creo nueva posicion en vectores
        sa_ID.push_back(ID_actual);
        sa_RN1.push_back(rn1); sa_RN2.push_back(rn2); sa_RN3.push_back(rn3);
        sa_PI1.push_back(pi1); sa_PI2.push_back(pi2); sa_PI3.push_back(pi3);
        sa_PF1.push_back(pf1); sa_PF2.push_back(pf2); sa_PF3.push_back(pf3);
    }
   
    
    */

}




int main_new() {
    
    
    //Creo vectores de prueba
    vector<long long int>Id_v;
    vector<int>rn1_v; vector<int>rn2_v; vector<int>rn3_v;
    vector<int>pi1_v; vector<int>pi2_v; vector<int>pi3_v;
    vector<int>pf1_v; vector<int>pf2_v; vector<int>pf3_v;

    //los relleno de prueba
    Id_v.push_back(1);
    
    rn1_v.push_back(1010101020202030303); rn2_v.push_back(1); rn3_v.push_back(1);
    pi1_v.push_back(11); pi2_v.push_back(11); pi3_v.push_back(11);
    pf1_v.push_back(1); pf2_v.push_back(1); pf3_v.push_back(1);

    //Creo base de datos
    const char* bbdd = "superbase01.db";
    db_crear(bbdd); //ESTO DA ERRROR

    //prueba de char a int PARA PONERLO EN EL CALLBACK

    const char* testaa = "23";
    std::string str(testaa);
    int numeroconvertido = std::stoi(str);
    std::cout << "numero convertido es: " << numeroconvertido << "\n";

    //Meto los array en la base de datos

    //Cierro la Base de datos

    //para que no se cierre
    int aaa111 = 0;
    std::cin >> aaa111;

    return 1;
}
int main_test_memoria_para_vector()
{
    //test vector memory allocation
    long long int num_of_items = 502512502512502512;

    //da error no soporta tanto espacio

    vector<int> myVector(num_of_items, 2);

    //for (int x : myVector)
    //    cout << x << " ";
    // Resultado es		2 2 2 2 2 
    return 1;
}
void main_test_crear_ID_actual() {
    //test para probar funcion crear_ID_actual
    long long int dado = 1000000000000000000;
    std::vector<int> v_dado = { 1,2,3,4,5,6,7,8,9 };
    long long int recibido = crear_ID_actual(dado, v_dado);
    

}
void main_TESTCREARID() 
//test para deshacer un long long int y ponerle un 1 mas por el medio
{

    std::cout << "comenzando" << endl;
    long long int lli_dado_1;   
    std::vector<int> v_dado_1;
    v_dado_1.clear();
    lli_dado_1 = 104010230102301023;
    v_dado_1 = { 20,10,5,20,10,5,20,10,5 };
    std::vector<int> v_dado_2 = { 0,2,1 };
    std::cout << crear_ID_actual_V2(lli_dado_1, v_dado_1,v_dado_2) << endl;
    return;
    
    ID_actual = 1193456123456123456;
    std::string str1; std::string str2;
    str1 = std::to_string(ID_actual);
    str2 = str1;
    int i;
    int j;
    std::istringstream(str1.substr(1, 2)) >> i;    
    i = i + 1;
    std::string inserto; inserto = std::to_string(i);
    if (inserto.size() < 2) {
        inserto = "0" + inserto;
    }
    str2.replace(1, 2, inserto);
    //str2[1] = '7';
    std::cout << "Numero extraido " << i << " inserto creado es "<< inserto << "\n";
    std::cout << "Cadena1 " << str1 << " y Cadena2 " << str2 <<"\n";
    ID_actual = std::stoll(str2);
    std::cout << "Long long int ya convertido " << ID_actual << "\n";
    
}
int main_random()
{
    //Genera numero true random
    // https://www.bogotobogo.com/cplusplus/RandomNumbers.php
    srand((int)time(0));
    int i = 0;
    while (i++ < 10) {
        int r = (rand() % 36) + 1;
        cout << r << " ";
    }
    return 0;
}
int main_par_impar() {
    int n;

    cout << "Enter an integer: ";
    cin >> n;

    if (n % 2 == 0)
        cout << n << " is par.";
    else
        cout << n << " is impar.";

    return 0;
}
int main_test_bola_ymas() {
    //test bola
    srand((int)time(0)); //TRUE RANDOM
    int ret_hist;
    std::vector<int> ristra_bolas;
    
    std::vector<int> repeticiones;
   
    for (int i = 0; i < 10; i++)
    {
        
        ristra_bolas.push_back(lanzar_bola()); 
        std::cout << "\n";
        std::cout << "ParImpar ";
        print_vector(hist_PI);
        ret_hist = actualizar_vectores_historicos(hist_PI);
        repeticiones.push_back(ret_hist);
        
        //std::cout << "RojoNegro ";
        //print_vector(hist_RN);  //dan problema por ser short int
        
        //std::cout << "PasaFalta ";
        //print_vector(hist_PF);
        std::cout << "repeticiones PI";
        print_vector(repeticiones);
        std::cout << "\n";
    }   
    /*
    std::cout << "Ristra Bolas";
    print_vector(ristra_bolas);
    std::cout << "RojoNegro";
    print_vector(hist_RN);  //dan problema por ser short int
    std::cout << "ParImpar";
    print_vector(hist_PI);
    std::cout << "PasaFalta";
    print_vector(hist_PF);
    */

    return 0;
}
int main_test_str_a_lli() {
    //main_test_str_a_lli
    long long int numeroconvertido;
    std::string str = "1000000000000000000";
    numeroconvertido = std::atoll(str.c_str());
    std::cout << numeroconvertido;
    return 0;
}


int main_sub()
{    
    bool debug_flag = false;
    //Pongo valor por defecto a variables globales   

    // agrego true random a al principio, si lo ponia dentro de cada dia, se repetian los mismo numeros
    //srand((int)time(0));

    //Creo variables de este proceso
    //int ret_int; //variable de RETorno para procesos que devuelven INT
    //const char* bbdd = "sb3.db"; //SETTING es el monbre de la base de datos

    //CREAMOS  LA DATABASE
    //ret_int = db_crear(bbdd);
    //print_vector(cb_vector);
    //db_crear(bbdd); //ESTO DA ERRROR

   
    //ret_int = test();    
    
    //std::cout << a1;
    //std::cout <<"tamano cd_vector es:" << cb_vector.size() << " contenido de cb_vector[0]:" << cb_vector[0] << "\n";
    
    /*
    //DEBUG LLENO VECTORES PARA PASAR A DB
    sa_ID = { 1000000000000000000,1010000010000010000,1001000001000001000 };
    sa_RN1 = { 1,0,0 }; sa_RN2 = { 0,1,0 }; sa_RN3 = { 0,0,1 };
    sa_PI1 = { 1,0,0 }; sa_PI2 = { 0,1,0 }; sa_PI3 = { 0,0,1 };
    sa_PF1 = { 1,0,0 }; sa_PF2 = { 0,1,0 }; sa_PF3 = { 0,0,1 };
    */

  
    //CREO las variables de retorno para que me retornen 0,1,2,3 DEL EXAMEN DE LOS VECTORES HISTORICOS  
    int ret_hist_RN; int ret_hist_PI; int ret_hist_PF;

    //Variables temporales de retorno con repeticiones hechas
    int RN_temp; int PI_temp; int PF_temp;


    //PARAMETROS DE LA SESION DE JUEGO  
    long long int j; //variable fija contador de bolas
    long long int idias = DIAS;
    int jbolas = BOLAS; //bolas minimas por sesion, se puede alargar si las reps estan a medias
    int max_bolas = 99; //maximo de volas por sesion

    //VARIABLES PARA EL CONTADOR DE TIEMPO
    int ciclodias = 0;
    

    //VARIABLE RETORNO EN QUE RECOJEMOS EL RESULTADO DE LA BOLA, SE USA SOLO COMO RETORNO, O PARA DEBUG
    int bola_que_sale;
    std::vector<int> ristra_bolas;  // se usa solo con debug_flag en true

    //VARIABLES FLGS
    bool flag_terminar;

    
    //pongo el contador
    std::cout << "Dias=" << idias << "\n";
    std::cout << "Bolas tiradas por dia=" << jbolas << " , mas las necesarias para ver futuras reps en ultima bola" << " \n";
    contador_comenzar(idias);

    //BUCLE PRINCIPAL DIAS
    for (long long int i = 0; i < idias; i++)    
    {               
        contador_actualizar(i);
        
        
        // 1 | xx (bola_n) | xx(RN1)|xx(RN2)|X(RN3)|xx(PI1)|xx(PI2)|x(PI3)|xx(PF1)|xx(PF2)|x(PF3)  
        ID_actual =100000000000000000 ;//112123451234512345; //EL 1 ESTA PARA QUE SALGAN LOS 0 DESPUES, TOTAL 18 DIGITOS
        

        //BUCLE BOLA AL DIA :EMPIEZO SESION DE JUEGO DE 1 DIA O 99 BOLAS
        //for (int j = 0; j < jbolas; j++)
        j = 1; flag_terminar = false;
        while(true)
        {       
            //veo si se han alcanzado  el numero de bola para la sesion
            if (j >= jbolas) { flag_terminar = true; }
            // 
            //pongo a 0 respuestas de los vectores historicos, ESTO NO HARIA FALTA SE PODRIA PONER ANTES DEL BUCLE
            ret_hist_RN = 0; ret_hist_PI = 0; ret_hist_PF = 0;

            //Lanzar bola , en la funcion ya se llenan los array historicos
            bola_que_sale = lanzar_bola();

            //actualizo la variable global de bola tirada, es 1 menos por que lo que sale es la predicion de la siguiente
            bola_n = j - 1;

            //debug para ver que bolas salen
            if (debug_flag == true) {
                ristra_bolas.push_back(bola_que_sale);
                std::cout << "Lanzo Bola Numero " << j << " y sale " << bola_que_sale << "\n";
                std::cout << "Ristra bolas ="; print_vector(ristra_bolas);// std::cout << endl;
            }
            
            //analizar los arrayshistoricos, ver que numero devuelven, las repeticiones que devuelven son limpiadas del historico ya
            ret_hist_RN = actualizar_vectores_historicos(hist_RN);
            //hist_PI.clear(); hist_PI = { 1,1,1};
            ret_hist_PI = actualizar_vectores_historicos(hist_PI);
            ret_hist_PF = actualizar_vectores_historicos(hist_PF);

            //debug print
            if (debug_flag == true) {
                std::cout << "Acualizar Vectores historicos PF devuelve " << ret_hist_PI << " en la cadena " << endl;
                std::cout << "Cadena Historica= ";
                print_vector(hist_PF);
                std::cout << endl;
            }
            /*
            //ESTO DEJO DE SER CONDICIONAL A SI SALIAN REPETICIONES NUEVAS POR QUE AHORA LO HAGO EN CADA BOLA QUE SE TIRA
            // NO SOLO SI SALEN NUEVAS REPS
            //se busca el condicional de si solo un array historico devuelve >0 se escribe en los vectores sa
            if (ret_hist_RN > 0 || ret_hist_PI > 0 || ret_hist_PF > 0)
            {
                //std::cout << "al menos un ret_hist_xx diferente a 0" <<"\n";               

                //Meter Vectores columnas xxx_sa, la funcion ya hace los pushback y busca si hay ID duplicado para no hacer mas grande el array
                actualizar_vectores_sa_XXX(ret_hist_RN, ret_hist_PI, ret_hist_PF);             
                
                 
            }
            */

            //ACUALIZO VECTORES temp_XXX, con las repeticiones nuevas que se detectan en la bola actual
            //Si esta en medio de unas reps ( numero negativo) pues le pongo quesea 0
            RN_temp = ret_hist_RN; PI_temp = ret_hist_PI; PF_temp = ret_hist_PF;
            if (RN_temp < 0) { RN_temp = 0; }
            if (PI_temp < 0) { PI_temp = 0; }
            if (PF_temp < 0) { PF_temp = 0; }

            //Meter Vectores columnas xxx_sa, la funcion ya hace los pushback y busca si hay ID duplicado para no hacer mas grande el array
            actualizar_vectores_temp_XXX(RN_temp, PI_temp, PF_temp);


            //termina la bola y vemos si se termina la sesion y hacemos break
            //std::cout << "bola que sale=" << bola_que_sale << endl;
            j = j + 1;
            if (j > jbolas) { flag_terminar = true; } // std::cout << "flag_terminar pasada a true" << endl; }
            if (flag_terminar == true)
            {
                // Tienen que ser o bien >0 (acaba de devolver una rep) o =0 (sigue repitiendo la misma rep)
                // Si es -xx significa que la rep se esta produciendo y no terminó                
                if (ret_hist_RN >= 0 && ret_hist_PI >= 0 && ret_hist_PF >= 0) { break; }
                if (j > max_bolas) { break; }
                //Salgo tb si estoy demasiado fuera del reps de bolas, es decir si voy 5 bolas más
                //esta claro que si hubiese habido una repeticion de 3 ya hubiese cantado, asi evito prolongaciones inecesarias
                if ((j - jbolas) > 5) { break; }
            }
        
        
        }
        //debug para ver que bolas salen
        if (debug_flag == true) {
            print_vector(ristra_bolas);
            ristra_bolas.clear();
        }
        //termina la sesion
        
        //BORRAR VECTORES SIN INFORMACION AL FUTURO, que se han creado para ver precisamente el futuro
        //en algun momento hay que parar, pero claro si quiero saber la informacion de 10 elementos y puede que 
        //tenga que tirar 3 o mas bolas para alante por que algo se va a repetir 3 o mas veces
        //esas posiciones de los vectores debe ser borradas
        //es jbolas-1, por que la ultima linea del vector es la que esta rompiendo las cadenas de historicos
        //y no tiene info completa de lo que saldra
        borrar_elementos_sobrantes_vectores_temp(jbolas-1); 

        //borro el hiatorial del dia
        hist_RN.clear(); hist_PI.clear(); hist_PF.clear();
        //std::cout << "Dia numero_" << i+1 << " terminado" << "\n";

        if (debug_flag == true) { print_10_vectores_temp(); }

        //anado vectores a los vectores xx_PREDB vectores predata base
        //Mezclo los vectores resultantes de la sesion con los sa_XX        
        
        mezclar_vectores_temp_y_sa();

        //borro vectores sa, hay que borralos cada dia por que si no al actulizar no se sabe cual es -1 -2 o -3
        borrar_vectores_temp();

        
        //aqui termina el dia
        
    }

    //ver vectores sa_xxx  (son la mezcla de todos los temporales)
    if (debug_flag == true) { print_10_vectores_sa(); }


    //salida debug
    //print_vector(ristra_bolas);
    
    //print_10_vectores_cb();
    //exit(314);

    //TODO ESTO LO VOY A PASAR A MAIN_VOLCAR() PARA PASAR EL SUPERARRAY A LA BASE DE DATOS
    //ACTUALIZO DB CON LOS ARRAYS
    //if (debug_flag == true) { std::cout << "Actualizando base de datos ... " << "\n"; }
    //ret_int = db_update_masivo(bbdd);
    //print_vector(cb_vector);
    //if (debug_flag == true) { std::cout << "Fin ... actualizacion base datos" << "\n"; }

    
    if (true) {


        //std::cin >> ret_int;
        contador_terminar();
        std::cout << sa_ID.size() << " Posiciones creadas en superarray" << endl;
        std::cout << "FIN DE LA CREACION DEL SUPERARRAY" << endl;
        
    }

    //BORRO VECTORES sa_XXX Y DEJO RESETEO BOLA
    //No hace falta borrar los vectores de momento solo cuando la memoria se llene y deje las pruebas
    //borrar_vectores_sa(); //NO PUEDO BORRAR LOS VECTORES SA
    borrar_vectores_cb();
    return 1;
}

int main_volcar() {
    //VOY PASANDO EL SUPER ARRAY A LA BASE DE DATOS

    //CREO BASE SI NO ESTA CREADA
    db_crear(DB_NOMBRE);


    //CONECTO CON BASE DE DATOS

    //ACTUALIZAR LA BASE DE DATOS EN UN BUCLE POR BLOQUES
    int i;
    int i_bloque = 900;

    double tamano_original = sa_ID.size();
    contador_comenzar(tamano_original);
    while (true) {
        //actualizo contador
        contador_actualizar(tamano_original - sa_ID.size());

        //COPIO UN VECTOR DE i_blouqe POSICIONES TEMPORAL
        borrar_vectores_temp();
        
        if (sa_ID.size() < i_bloque) { i_bloque = sa_ID.size(); } //esto es para que si el vector es < i_bloque
        for (i = 0; i < i_bloque; i++) {
            temp_ID.push_back(sa_ID[i]); temp_IDREPS.push_back(sa_IDREPS[i]);
            temp_RN1.push_back(sa_RN1[i]); temp_RN2.push_back(sa_RN2[i]); temp_RN3.push_back(sa_RN3[i]);
            temp_PI1.push_back(sa_PI1[i]); temp_PI2.push_back(sa_PI2[i]); temp_PI3.push_back(sa_PI3[i]);
            temp_PF1.push_back(sa_PF1[i]); temp_PF2.push_back(sa_PF2[i]); temp_PF3.push_back(sa_PF3[i]);            
        }
        
        //print_10_vectores_temp();
       


        //LO MANDO A ACTUALIZAR LA BASE DE DATOS
        db_update_masivo(DB_NOMBRE);

        //BORRO LAS POSICIONES COPIADAS AL TEMPORAL DESDE DEL SUPERVECTOR
        sa_ID.erase(sa_ID.begin(), sa_ID.begin() + i_bloque); sa_IDREPS.erase(sa_IDREPS.begin(), sa_IDREPS.begin() + i_bloque);
        sa_RN1.erase(sa_RN1.begin(), sa_RN1.begin() + i_bloque); sa_RN2.erase(sa_RN2.begin(), sa_RN2.begin() + i_bloque); sa_RN3.erase(sa_RN3.begin(), sa_RN3.begin() + i_bloque);
        sa_PI1.erase(sa_PI1.begin(), sa_PI1.begin() + i_bloque); sa_PI2.erase(sa_PI2.begin(), sa_PI2.begin() + i_bloque); sa_PI3.erase(sa_PI3.begin(), sa_PI3.begin() + i_bloque);
        sa_PF1.erase(sa_PF1.begin(), sa_PF1.begin() + i_bloque); sa_PF2.erase(sa_PF2.begin(), sa_PF2.begin() + i_bloque); sa_PF3.erase(sa_PF3.begin(), sa_PF3.begin() + i_bloque);

        //SALGO SI EL VECTOR sa_xxx ya esta vacio
        if (sa_ID.size() == 0) { break; }        
    }

    //DESCONECTO CON BASE DE DATOS

    
    //contador imprimo now
    contador_terminar();

    return 1;
}

int main() {
    // agrego true random a al principio, si lo ponia dentro de cada dia, se repetian los mismo numeros
    srand((int)time(0));

    long long int veces;
    int bolas;
    std::cout << "Cuantas bolas se tiran por sesion?";
    std::cin >> bolas;
    BOLAS = bolas + 2;
    std::cout << "Cuantas veces ejecutamos la rutina de " << bolas << " bolas?" << endl;
    std::cout << "Tarda unos 5 minutos cada 100000" << endl;
    std::cin >> veces ;
    DIAS = veces;
    
    int a;
    std::cout << "Empiezo proceso de generear super array y pasarlo a la base de datos " << endl;
    a = main_sub();       
    std::cout << "Proceso terminado" << endl;
    std::cout << " " << endl;    


    std::cout << "Comienzo volcado a base de datos" << endl;
    a=main_volcar();
    //Mensaje de fin
    std::cout << "Volcado a base de datos terminado, podemos cerrar la ventana" << endl;    
    std::cin >> a;
    return 1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
