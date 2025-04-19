# IMPORTANTE

Para correr la prueba de gramatica asegurese de que el archivo `tasks.json`, en la carpeta `.vscode`, si esta usando el visual studio code, contenga la linea: `"command": "/usr/bin/g++",`. Normalmente viene con: `"command": "/usr/bin/gcc",`. Ademas, asegurese de incluir en el mismo archivo, en los `args`, y entre `""`, cada archivo .cpp junto a la ruta de este en el proyecto. El compilador de c++ da error si no se hace esto. Una vez cumplidas estas medidas, deberia poder ejecutar su archivo main al presionar `F5` en este.
