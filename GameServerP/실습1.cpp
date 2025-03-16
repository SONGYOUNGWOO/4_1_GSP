#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;
int main()
{
	volatile long long tmp = 0;

	auto start = high_resolution_clock::now();
	for (int j = 0; j < 10000000; ++j) {
		tmp += j;
		//this_thread::yield();
	}
	auto duration = high_resolution_clock::now() - start;
	cout << "Time " << duration_cast<milliseconds>(duration).count();
	cout << " msec\n";
	cout << "RESULT " << tmp << endl;

	// Time 3 msec
	// 운영체제 실행 (스레드를 양보하면서 실행)
	// Time 826 msec , 300배 차이
	// 디버그 메시지 출력 하지말자 , 하더라도 최종 빌드에서는 출력하지 않도록 하자
}