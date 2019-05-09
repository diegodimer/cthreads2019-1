
|setIteratorToFirst|Detalhes|
|--|--|
|@brief|Seta de uma vez só todos os iteradores de todas as filas para o primeiro elemento|
|@retval|None|


|scheduler|Detalhes|
|--|--|
|@brief|Função responsável pela manutenção das filas da biblioteca. É para onde a thread em execução vai quando precisa ser bloqueada, terminada ou ceder a CPU.|
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
|@brief|inicializa a fila e o contador do semáforo|
|@param|*sem: ponteiro para a estrutura do semáforo|
|@param|count: número de threads que podem usar o recurso|
|@retval|-1 se houve um problema na criação da fila do semáforo, 0 caso contrário||



 
|cwait|Detalhes|
|--|--|
|@brief|solicita um recurso, se count <= 0 a thread é bloqueada e colocada na fila de espera do semáforo|
|@param|*sem: ponteiro para a estrutura do semáforo|
|@retval|-1 se o ponteiro do semáforo é NULL, -3 se não conseguir colocar a thread na fila do semáforo, 0 se não houve erros||



 
|csignal|Detalhes|
|--|--|
|@brief| incrementa o sem->count para indicar que um recurso foi desalocado, tira uma thread da fila do semáforo e a coloca em uma fila de aptos, remove essa thread da fila dos bloqueados|
|@param|*sem: ponteiro para a estrutura do semáforo |
|@retval|-2 se não conseguiu inserir a thread na fila de aptos, o se não houve erros||



 
|cidentify|Detalhes|
|--|--|
|@brief|Identificação dos integrantes do grupo|
|@param|name: ponteiro para string a ser preenchida com nome dos integrantes do grupo|
|@param|size: tamanho da string no ponteiro name|
|@retval|||
