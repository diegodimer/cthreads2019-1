
|setIteratorToFirst|Detalhes|
|--|--|
|@brief|Seta de uma vez só todos os iteradores de todas as filas para o primeiro elemento|
|@retval|None|


|scheduler|Detalhes|
|--|--|
|@brief|Função responsável pela manutenção das filas da biblioteca. É para onde as thread em execução vai quando precisa ser bloqueada, terminada ou ceder a CPU.|
|@retval|None|


|initQueues|Detalhes|
|--|--|
|@brief|Inicializa todas as filas e lida com a thread main, alocando uma estrutura do tipo TCB_t para ela e criando um contexto.|
|@retval|None|

 
|ccreate|Detalhes|
|--|--|
|@brief|Criação de uma thread|
|@param|void *(*start)(void *): ponteiro para função que a thread executará|
|@param|arg: parâmetro a ser usado por essa função|
|@param|prio: prioridade da thread, 0: alta, 1: média, 2: baixa|
|@retval|inteiro positivo caso tenha conseguido criar a thread, -1 caso contrário|

 
|csetprio|Detalhes|
|--|--|
|@brief|Troca a prioridade da thread em execução|
|@param|tid: NULL |
|@param|prio: nova prioridade da thread em execução|
|@retval|0 se conseguiu, -1 caso contrário|

 
|cyield|Detalhes|
|--|--|
|@brief|Cedência voluntária, muda o estado da thread em execução para apto|
|@retval|0 caso conseguiu, -1 caso contrário|

 
|cjoin|Detalhes|
|--|--|
|@brief|Sincronização de término, bloqueia a thread em execução até o termino da thread com id informado|
|@param|tid: id da thread a ser esperada|
|@retval|0 caso sucesso, -1 caso não encontrou a thread que deseja esperar, -2 se já tem alguem esperando pela thread que deseja esperar|

 
|csem_init|Detalhes|
|--|--|
|@brief||
|@param|sem: |
|@param|count: |
|@retval|||



 
|cwait|Detalhes|
|--|--|
|@brief||
|@param|sem: |
|@retval|||



 
|csignal|Detalhes|
|--|--|
|@brief||
|@param|sem: |
|@retval|||



 
|cidentify|Detalhes|
|--|--|
|@brief|Identificação dos integrantes do grupo|
|@param|name: ponteiro para string a ser preenchida com nome dos integrantes do grupo|
|@param|size: tamanho da string no ponteiro name|
|@retval|||
