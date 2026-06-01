#include <thread>
#include <stdio.h>
#include <unistd.h>

volatile bool active = true;
pthread_mutex_t m;
int total = 0;

void myfunc() {
	while (active) {
		pthread_mutex_lock(&m);
		total++;
		pthread_mutex_unlock(&m);
	}
}

int main(int, char**) {
	pthread_mutex_init(&m, nullptr);
	std::thread thds[8];
	for (auto & t : thds)
		t = std::thread(myfunc);
	sleep(1);
	active = false;
	for (auto & t : thds)
		t.join();
	printf("total %d\n", total);
}
