// Link del github : https://github.com/Emii09/Telegram_BOT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

struct memory {
  char *response;
  size_t size;
};

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = nmemb;
  struct memory *mem = clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(!ptr)
    return 0;  /* out of memory */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

int main(int argc, char* argv[])
{
	if (argc < 2){
		printf("\nERROR, envie la cantidad de archivos necesarios");
		return 1;
	}
	
	FILE* f = fopen(argv[1],"r");
	if(!f){
		printf("\nERROR, no se pudo abrir el archivo del TOKEN");
		return 1;
	}
	
	char token[200];
	fgets(token, sizeof(token),f);
	token[strcspn(token, "\n")] = 0;
	fclose(f);
	
	long long offset = 0;

  CURLcode res;
  CURL *curl = curl_easy_init();

  while (1){

	printf("\n");
	
	char api_url[500];
	sprintf(api_url, "https://api.telegram.org/bot%s/getUpdates?offset=%lld", token, offset);
	
	struct memory chunk = {0};
	
	curl_easy_setopt(curl, CURLOPT_URL, api_url);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	
	res = curl_easy_perform(curl);
	
	if (res != 0)
		printf("Error Codigo: %d\n", res);
	
	if (!chunk.response){
		sleep(2);     
		continue;    
	}
	
	printf("%s\n", chunk.response);
	
	char* ult_update = chunk.response;
	char* q = chunk.response;
	
	while ((q = strstr(q, "\"update_id\":")) != NULL){
		ult_update = q;
		q++;
	}
	
	if (strstr(chunk.response, "\"result\":[]")){
		printf("\nNo hay mensajes nuevos.\n");
		
		free(chunk.response);
		sleep(2);  
		continue; 
	}
	
	long long update_id = 0;
	sscanf(ult_update,"\"update_id\":%lld", &update_id );
	printf("\nupdate_id = %lld\n", update_id);
	
	long long chat_id = 0;
	char* p = strstr (ult_update, "\"chat\":" );
	if (p){
		p = strstr(p, "\"id\":");
		sscanf(p , "\"id\":%lld", &chat_id);
		printf("\nchat_id = %lld\n", chat_id);
	}
	
	char nombre [100] = {0};
	p = strstr (ult_update, "\"first_name\":");
	if (p){
		sscanf (p, "\"first_name\":\"%[^\"]\"", nombre);
		printf("nombre = %s\n", nombre);
	}
	
	char mensaje[200] = {0};
	p = strstr (ult_update, "\"text\":");
	if (p){
		sscanf(p, "\"text\":\"%[^\"]\"", mensaje);
		printf("mensaje = %s\n", mensaje);
	}
	
	long long tiempo = 0;
	p = strstr(ult_update, "\"date\":");
	if (p) {
		sscanf(p, "\"date\":%lld", &tiempo);
	}
	printf("tiempo = %lld\n", tiempo);
	
	char send_url[600];
	
	if (strcmp(mensaje, "hola") == 0) {
	
		sprintf(send_url, "https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Hola%%20%s",token, chat_id, nombre);
		
	}else if (strcmp(mensaje, "estoy aprobado?") == 0) {
		
		sprintf(send_url,"https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Si!%%20decile%%20a%%20tu%%20profe%%20que%%20te%%20deje%%20ir",token, chat_id);
		
	}else if(strcmp(mensaje, "chau")== 0){
		
		sprintf(send_url,"https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Chau%%20%s%%2C%%20nos%%20vemos%%20luego.",token, chat_id, nombre);
		
		
	}else {
		free(chunk.response);
		offset = update_id + 1;
		sleep(2);

		continue;
	}
	
	FILE* msj1 = fopen("registro.txt", "a");
	if (msj1){
		
		fprintf(msj1, "(%s) = \"%s\" (TIME = %lld)\n", nombre, mensaje, tiempo);
		fclose(msj1);
		
	}
	
	FILE* msj2 = fopen("registro.txt", "a");
	if (msj2){
		
		if (strcmp(mensaje, "hola") == 0){
			
			fprintf(msj2, "(bot) = \"Hola %s\" (TIME = %lld)\n", nombre, tiempo);
			fprintf(msj2,"\n");
			
		}else if (strcmp(mensaje, "estoy aprobado?") == 0){
			
			fprintf(msj2, "(bot) = \"Si! decile a tu profe que te deje ir\" (TIME = %lld)\n", tiempo);
			fprintf(msj2,"\n");
			
		}else if (strcmp(mensaje, "chau") == 0){
			fprintf(msj2, "(bot) = \"Chau %s, nos vemos luego.\" (TIME = %lld)\n", nombre, tiempo);
			fprintf(msj2, "\n");
		}
		
		fclose(msj2);
	}
	
	free(chunk.response);
	
	struct memory chunk2 = {0};
	curl_easy_setopt(curl, CURLOPT_URL, send_url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);
	
	res = curl_easy_perform(curl);
	
	if (res != 0)
		printf("Error al enviar: %d\n", res);
	else
		printf("\nRespuesta enviada\n");

	free(chunk2.response);
	
	offset = update_id + 1;
	
	sleep(2);
  }
  
  curl_easy_cleanup(curl);
  return 0;
}
