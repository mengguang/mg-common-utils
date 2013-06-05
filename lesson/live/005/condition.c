#include <stdio.h>
#include <stdbool.h>

_Bool isHoliday(int day) {
	if(day == 6 || day == 7) {
		return true;
	} else {
		return false;
	}
}

void print_day(int day) {

	switch(day) {
		case 1:
			printf("Monday\n");
			break;
		case 2:
			printf("Tuesday\n");
			break;
		case 3:
			printf("Wednesday\n");
			break;
		case 4:
			printf("Thursday\n");
			break;
		case 5:
			printf("Friday\n");
			break;
		case 6:
			printf("Saturday\n");
			break;
		case 7:
			printf("Sunday\n");
			break;
		default:
			printf("Unknown\n");
			break;
	}
	return;

}

int main(int argc,char **argv) {

	_Bool holiday = true;

	int day=6;
	if(isHoliday(day)) {
		printf("%d is holiday.\n",day);
	} else {
		printf("%d is not holiday.\n",day);
	}
	print_day(1);
	print_day(2);
	print_day(3);
	print_day(4);
	print_day(5);
	print_day(6);
	print_day(7);

	return 0;
}

