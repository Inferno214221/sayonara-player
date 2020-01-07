#include <utility>

std::pair<int, double> get_pair()
{
	return std::pair(1, 4.5);
}

int main()
{
	auto [a, b] = get_pair();
	return a;
}
