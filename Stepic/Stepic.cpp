#include <iostream>
#include <unordered_map>

// Класс хранения свойств числа
class number_properties 
{
protected :
	int fresh; // свежесть числа
	int counter; // счетчик числа
public :
	number_properties() : fresh(0), counter(0)
	{}
	number_properties(int fresh, int counter) : fresh(fresh), counter(counter)
	{}
	// Метод проверки на несвежесть числа
	inline bool Not_fresh() const
	{
		return fresh < 0;
	}
	// Метод обновления счетчика при добавлении числа
	inline void Update_counter()
	{
		++counter;
		++fresh;
	}
	// Метод деградирования свежести числа
	inline void Degrede_fresh()
	{
		--fresh;
	}
	bool operator == (const number_properties& rValue) const
	{
		return fresh == rValue.fresh && counter == rValue.counter;
	}
	bool operator < (const number_properties& rValue) const
	{
		return fresh < rValue.fresh ? true : // либо самое старое, либо, при одинаковой свежести,
			fresh == rValue.fresh ? counter < rValue.counter : false; // самое редкое
	}
};

// Класс подсчета повторений чисел
class number_counter
{
private :
	const unsigned SIZE_MAP; // итоговый размер выходного множества
	long long tick; 

	std::unordered_map<int, number_properties> numbers_buf; // буфер, хранящий числа и их свойства
	std::unordered_map<int, number_properties>::iterator iter; // Итератор обхода по буферу
	std::unordered_map<int, number_properties>::iterator end; // Итератор конца буфера
	std::unordered_map<int, number_properties>::iterator erase; // Итератор на удаление числа из буфера

	// Метод удаления лишних чисел из буфера
	void Remove_unnecessary()
	{
		number_properties prev_min_fresh(LONG_MAX, LONG_MAX); // Для жранения предыдущего минимума под удаления
		
		// первый проход в лоб, пробуем управиться удалением только протухших чисел
		iter = numbers_buf.begin();
		end = numbers_buf.end();
		while (iter != end && numbers_buf.size() > SIZE_MAP)
			if (iter->second.Not_fresh())
			{ // удаляем протухшее число
				iter = numbers_buf.erase(iter);
			}
			else
			{	// попутно ищем минимум
				if (iter->second < prev_min_fresh)
					prev_min_fresh = iter->second;
				++iter;
			}

		// второй проход в лоб, пробуем управиться удалением только самых старых чисел
		iter = numbers_buf.begin();
		end = numbers_buf.end();
		while (iter != end && numbers_buf.size() > SIZE_MAP)
			if (iter->second == prev_min_fresh) // удаляем самое старое число
				iter = numbers_buf.erase(iter);
			else
				++iter;

		// самый сложный проход, с поиском минимальных значений каждую итерацию
		while (numbers_buf.size() > SIZE_MAP)
		{
			iter = numbers_buf.begin();
			end = numbers_buf.end();
			erase = iter;
			number_properties min_fresh = iter->second;

			// уже был найден этот мининимум в предыдущей итерации
			if (prev_min_fresh == min_fresh) 
			{
				numbers_buf.erase(erase);
				continue; // приемлемо остановиться на этом числе
			}

			bool find_min = true; // флаг нахождения минимума
			// ищем самое старое число
			for (; iter != end; ++iter)
				if (iter->second < min_fresh) // если нашли число старее
				{
					min_fresh = iter->second;
					erase = iter;

					// уже был найден этот мининимум в предыдущей итерации
					if (prev_min_fresh == min_fresh) 
					{
						find_min = false; // новый минимум не был найден
						break; // приемлемо остановиться на этот числе
					}
				}
			
			// обновляем минимум
			if (find_min)
				prev_min_fresh = min_fresh;
			
			// и удаляем число
			numbers_buf.erase(erase);
		}
	}
public :
	number_counter(unsigned SIZE_MAP) : SIZE_MAP(SIZE_MAP), tick(0)
	{}
	// Метод добавления числа 
	void Step(int number)
	{
		numbers_buf[number].Update_counter(); // добавляем число
		
		// на итерации кратной количеству чисел в ответе
		if (++tick % SIZE_MAP == 0)
		{
			iter = numbers_buf.begin();
			end = numbers_buf.end();
			for (; iter != end; ++iter)
				iter->second.Degrede_fresh(); // числа стареют

			// удаляем лишнее
			if (numbers_buf.size() > SIZE_MAP)
				Remove_unnecessary();
		}
	}
	// метод вывода имеющихся чисел в поток вывода
	void Print()
	{
		// удаляем лишнее
		if (numbers_buf.size() > SIZE_MAP)
			Remove_unnecessary();

		iter = numbers_buf.begin();
		end = numbers_buf.end();
		for (; iter != end; ++iter)
			std::cout << iter->first << ' '; // выводим числа
	}
};

int main()
{
	// создаем объект хранения, задавая параметр согласно условию: кол-во чисел в ответе (1000)
	number_counter counter(1000); 
	int number = 0; // временное храниние принимаемого числа

	while (std::cin >> number)
		counter.Step(number);
	
	counter.Print();

	return 0;
}