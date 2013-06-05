#include <stdio.h>

int main(int argc,char **argv) {
	
	struct Person {
		int id;
		int age;
		char *name;
		char *email;
	};

	struct Person p1;
	p1.id=1;
	p1.age=18;
	p1.name="laomeng";
	p1.email="laomeng188@163.com";

	printf("id = %d, age=%d, name=%s, email=%s\n",p1.id,p1.age,p1.name,p1.email);

	printf("sizeof struct Person: %d\n",sizeof(struct Person));
	
	return 0;
}
