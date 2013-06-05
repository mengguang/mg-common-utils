#include <stdio.h>
#include <stdbool.h>

void print_day(int day) {
	switch(day) {
		case 1:
			printf("Today is Monday.\n");
			break;
		case 2:
			printf("Today is Tuesday.\n");
			break;
		case 3:
			printf("Today is Wednesday.\n");
			break;
		case 4:
			printf("Today is Thursday.\n");
			break;
		case 5:
			printf("Today is Friday.\n");
			break;
		case 6:
			printf("Today is Saturday.\n");
			break;
		case 7:
			printf("Today is Sunday.\n");
			break;
		default:
			printf("Today is Unknown Day.\n");
			break;
	}
	return;

}

_Bool isHoliday(int day) {
	if(day == 6 || day == 7) {
		return true;
	} else {
		return false;
	}
}

int main(int argc,char **argv) {

	_Bool holiday = isHoliday(7);
	if(holiday) {
		printf("Today is holiday.\n");
	} else {
		printf("Today is not holiday.\n");
	}

	int age=30;

	if(age > 18) {
		printf("you are old enough to get a work.\n");
	} else {
		printf("you should go to school.\n");
	}


	int day=6;
	switch(day) {
		case 1:
			printf("Today is Monday.\n");
			break;
		case 2:
			printf("Today is Tuesday.\n");
			break;
		case 3:
			printf("Today is Wednesday.\n");
			break;
		case 4:
			printf("Today is Thursday.\n");
			break;
		case 5:
			printf("Today is Friday.\n");
			break;
		case 6:
			printf("Today is Saturday.\n");
			break;
		case 7:
			printf("Today is Sunday.\n");
			break;
		default:
			printf("Today is Unknown Day.\n");
			break;
	}

	for(day=0;day<10;day++) {
		print_day(day);
	}


}
