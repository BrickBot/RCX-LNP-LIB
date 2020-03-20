/* talker.c */

#include <conio.h>
#include <unistd.h>
#include <string.h>
#include <lnp.h>
#include <dsensor.h>
#include <dmotor.h>
#include <time.h>
#include <sys/time.h>
#include <sys/battery.h>

static int quit = 0;

void my_integrity_handler(const unsigned char *data, unsigned char len)
{
	char msg[7];
	int i;

	if (len > 5) len = 5;
	for (i = 0; (i < len); i++) msg[i] = data[i];
	msg[i] = '\0';
	cputs(msg);

//handle_message(msg);

if (strcmp(msg,"bat")==0)
{
int bat = get_battery_mv();
cputw(bat);
}

//////////////////////////////////
if (strcmp(msg,"m1")==0)
{
cputs("ok m1");
motor_a_speed(MAX_SPEED);
motor_a_dir(fwd);
}

if (strcmp(msg,"m2")==0)
{
cputs("ok m2");
motor_b_speed(MAX_SPEED);
motor_b_dir(fwd);
}

if (strcmp(msg,"m3")==0)
{
cputs("ok m3");
motor_c_speed(MAX_SPEED);
motor_c_dir(fwd);
}

//////////////////////////////////
if (strcmp(msg,"rev1")==0)
{
cputs("rev1 ");
motor_a_dir(rev);
}

if (strcmp(msg,"rev2")==0)
{
cputs("rev 2");
motor_b_dir(rev);
}

if (strcmp(msg,"rev3")==0)
{
cputs("rev 3");
motor_c_dir(rev);
}

//////////////////////////////////
if (strcmp(msg,"stop1")==0)
{
cputs("stop1");
motor_a_dir(brake);
}

if (strcmp(msg,"stop2")==0)
{
cputs("stop2");
motor_b_dir(brake);
}

if (strcmp(msg,"stop3")==0)
{
cputs("stop3");
motor_c_dir(brake);
}

//////////////////////////////////

if (strcmp(msg,"stopa")==0)
{
cputs("stopa");
motor_a_dir(brake);
motor_b_dir(brake);
motor_c_dir(brake);
}




//	handle_message(msg);
	if (data[0] == 'q') quit = 1;
}


int main()
{
	char *s = "msg   ";
	int loop = 0;
	lnp_integrity_set_handler(my_integrity_handler);
	cputs("ready");
	while(!quit)
	{
 if(SENSOR_1 != 0)
{
//motor_c_speed(MAX_SPEED); 		
//motor_c_dir(fwd);
//cputs("s1 ok");
}
s[4] = '0' + loop / 10;
		s[5] = '0' + loop % 10;
		lnp_integrity_write(s,strlen(s));
		msleep(500);
		loop = (loop + 1) % 100;
	}
	return 0;
}




/*

int handle_message(char msg)
{
if (strcmp(msg,"func")==0)
{
 cputs("func");
}

return 0;
}

*/