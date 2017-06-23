#include <iostream>
#include <cstdint>
#include <vector>

template<unsigned int Bits>
class BitVector
{
public:
	BitVector(size_t capacity)
		: m_size(0)
	{
		m_elements.reserve((capacity * Bits) / 64);
		m_elements.push_back(0);
	}

	void push_back(uint64_t value)
	{
		auto bitCount = m_size * Bits;
		auto current = bitCount % 64;

		++m_size;
		m_elements.back() |= value << current;
		auto overflow = current + Bits;
		if (overflow >= 64)
		{
			m_elements.push_back(0);
			m_elements.back() = value >> (Bits - (overflow - 64));
		}
	}

	uint64_t operator[](size_t pos) const
	{
		auto index = pos * Bits;
		auto elementIndex = index / 64;
		
		auto current = index % 64;
		auto end = current + Bits;
		if (end <= 64)
		{
			auto offset = 64 - end;
			auto re = m_elements[elementIndex] << offset;
			return re >> (offset + current);
		}

		auto re = m_elements[elementIndex] >> current;
		auto next = Bits - (64 - current);
		auto sec = (m_elements[elementIndex + 1] << (64 - next)) >> (64 - next);
		return re + (sec << (Bits - next));
	}

	std::vector<bool> search(uint64_t value) const
	{
		return std::vector<bool>();
	}

private:

	size_t m_size;
	std::vector<uint64_t> m_elements;

	static_assert(Bits <= 64, "Yo");
};

int main()
{
	{
		BitVector<32> yo(0);

		yo.push_back(1);
		yo.push_back(2);
		yo.push_back(3);
		yo.push_back(4);

		for (size_t i = 0; i < 4; ++i)
		{
			std::cout << yo[i] << " ";
		}
		std::cout << std::endl;
	}

	{
		BitVector<9> yo(12);

		for (size_t i = 0; i < 20; ++i)
		{
			yo.push_back(i);
		}

		for (size_t i = 0; i < 20; ++i)
		{
			std::cout << yo[i] << " ";
		}
		std::cout << std::endl;
	}

	return 0;
}