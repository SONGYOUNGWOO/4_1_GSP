#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

constexpr int T_SIZE = 100000000;
short rand_arr[T_SIZE];


#define abs1(x) (((x)>0)?(x):-(x)) // >0)? - 찍은게 틀리면  파이프라인 리셋 

int abs2(int x) // 2배빠름 
{
	int y = x >> 31;
	return (y ^ x) - y;

}
int main()
{
	for (int i = 0; i < T_SIZE; ++i) rand_arr[i] = rand() - 16384;
	int sum = 0;
	auto start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs1(rand_arr[i]);
	auto du = high_resolution_clock::now() - start_t;
	cout << "[abs1] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
	sum = 0;
	start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs2(rand_arr[i]);
	du = high_resolution_clock::now() - start_t;
	cout << "[abs2] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
}