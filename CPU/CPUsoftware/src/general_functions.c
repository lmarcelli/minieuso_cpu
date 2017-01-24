#include "globals.h"

/* create log file */
void create_log(void)
{
	char logname[40];
	time_t now = time(NULL);
	struct tm *t;
	t = gmtime(&now);

	strftime(logname, sizeof(logname), "../log/CPU_MAIN_%Y-%m-%d_%H:%M:%S.log", t);
	log_file = fopen(logname, "a");
}

/* logging function */
void tlog(char * tag, char * message) 
{
	if (log_file != NULL) 
	{
		//time_t ltime; /* calendar time */
    	//ltime=time(NULL); /* get current cal time */
    	char buf[40];
    	time_t now = time(NULL);
		struct tm *t;
		t = gmtime(&now);

		//fprintf(log_file,"%s: %s: %s\n",asctime(localtime(&ltime)),tag,message);
		strftime(buf, sizeof(buf), "%Y-%m-%d_%H:%M:%S", t);
		fprintf(log_file,"%s: [%s] %s\n",buf,tag,message);
		//fflush(log_file); //flush otherwise will be lost if an error occurs

		if ((tag[0] == 'E') && (tag[1] == 'R') && (tag[2] == 'R')) 
		{
			//printf("%s: [%s] %s\n",asctime(localtime(&ltime)),tag,message);
		}
	}
	else 
	{
			printf("Error in tlog(): log_file not open\n");
			system("pause");
	}

}