# RouletteCombinations
C++ code for a roulette strategy with outcome outsourced in sqlite database
El año pasado programé una estrategia de ruleta rojo/negro en C++, el objetivo era aprender algo de SQL(SQLite) y de paso ver si había alguna ventaja en la estrategia,
solo sirvió para descubrir que  la función rand() de C++ no es 100% aleatoria, comparto codigo por si a alguien le sirve
El ejecutable crea una db.db, que es base da datos sqlite, en esa base de datos pude ver que habia ciertas ventajas no lógicas que solo responden a que 
la funcion rand() se C++ tiene sus deficiencias, osea no es 100% random
