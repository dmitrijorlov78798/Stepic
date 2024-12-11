#include <iostream>
#include <unordered_map>

/// <summary>
/// Класс, хранящий свойства числа: число повторений и свежесть.
/// Применение счетчика числа интуитивно понятно, а вот свежесть введена
/// для случая когда буфер заполнен числами с одинаковым небольшим счетчиком, которые
/// не дают задержаться в буфере новым числа (среди которых, возможно находятся целевые числа).
/// Поэтому с течением времени свежесть деградирует, и число с отрицательной свежестью может
/// быть удалено с буфера без поиска самого старого/редкого числа (в буфере размером 1к, число 
/// встречающееся 1к раз при входных данных 1м будут иметь свежесть - 0, и не удалится)
/// </summary>
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
	// Метод возвращает признак несвежести числа
	inline bool Not_fresh() const
	{
		return fresh < 0; // возвращаем, не протухло ли число
	}
	// Метод обновления счетчика при добавлении числа
	inline void Update_counter()
	{
		fresh = ++counter; // При обновлении счетчика, выставляем свежесть равным ему
	}
	// Метод деградации свежести числа
	inline void Degrede_fresh(int target_count_repet)
	{
		if (counter < target_count_repet) // если число уже подходит под условия, свежесть не деградирует
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

/// <summary>
/// Класс адаптер над unordered_map для подсчета повторений чисел.
/// Идея состоит в использовании ограниченом по размеру буферу с числами,
/// где они со временем без обновления стареют, и могут быть вытеснены более
/// свежими и частыми числами.
/// </summary>
class number_counter
{
private :
	const unsigned SIZE_MAP; // итоговый размер выходного множества
	const int TARGET_COUNT_REPET; // целевое количество повторений числа
	long long tick; 

	std::unordered_map<int, number_properties> numbers_buf; // буфер, хранящий числа и их свойства

	// Метод удаления лишних чисел из буфера
	void Remove_unnecessary()
	{
		number_properties prev_min_fresh(LONG_MAX, LONG_MAX); // Для жранения предыдущего минимума под удаления
		
		// первый проход в лоб, пробуем управиться удалением только протухших чисел 
		for (auto iter = numbers_buf.begin(); iter != numbers_buf.end() && numbers_buf.size() > SIZE_MAP; )
			if (iter->second.Not_fresh())
			{ // удаляем протухшее число
				iter = numbers_buf.erase(iter);
			}
			else
			{	// попутно ищем минимум (самое старое/редкое)
				if (iter->second < prev_min_fresh)
					prev_min_fresh = iter->second;
				++iter;
			}

		// второй проход в лоб, пробуем управиться удалением только самых старых чисел
		for (auto iter = numbers_buf.begin(); iter != numbers_buf.end() && numbers_buf.size() > SIZE_MAP; )
			if (iter->second == prev_min_fresh) // удаляем самое старое число
				iter = numbers_buf.erase(iter);
			else
				++iter;

		// Для худшего случая с уникальными числами на входе должны управиться за проходы в лоб 
		// Если не управились, то самый сложный проход, с поиском минимальных значений каждую итерацию
		while (numbers_buf.size() > SIZE_MAP)
		{
			auto iter = numbers_buf.begin();
			auto end = numbers_buf.end();
			auto erase = iter;
			number_properties min_fresh = iter->second;

			// уже был найден этот мининимум в предыдущей итерации
			if (prev_min_fresh == min_fresh) 
			{
				numbers_buf.erase(erase);
				continue; // приемлемо остановиться на этом числе
			}

			// ищем самое старое число
			for (; iter != end; ++iter)
				if (iter->second < min_fresh) // если нашли число старее
				{
					min_fresh = iter->second;
					erase = iter;
					// уже был найден этот мининимум в предыдущей итерации
					if (prev_min_fresh == min_fresh) 
						break; // приемлемо остановиться на этот числе
				}
			
			prev_min_fresh = min_fresh;
			numbers_buf.erase(erase);
		}
	}
public :
	number_counter(unsigned SIZE_MAP, int TARGET_COUNT_REPET) : SIZE_MAP(SIZE_MAP), TARGET_COUNT_REPET(TARGET_COUNT_REPET), tick(0)
	{
		numbers_buf.reserve(2 * SIZE_MAP + 1); // выделяем с запасом
	}
	// Метод добавления числа 
	void Step(int number)
	{
		numbers_buf[number].Update_counter(); // добавляем число
		// Каждую итерацию, кратную размеру буфера
		if (++tick % SIZE_MAP == 0) 
			for (auto iter = numbers_buf.begin(); iter != numbers_buf.end(); ++iter)
				iter->second.Degrede_fresh(TARGET_COUNT_REPET); // числа стареют
		// Если количество чисел больше предела взятого с запасом (но не превышающего допустимого)
		if (2 * SIZE_MAP <= numbers_buf.size())
			Remove_unnecessary(); // удаляем лишнее
	}
	// метод вывода имеющихся чисел в поток вывода
	void Print()
	{
		// удаляем лишнее, приводя размер буфера к целлевому ответу
		if (numbers_buf.size() > SIZE_MAP)
			Remove_unnecessary();

		for (auto iter = numbers_buf.begin(); iter != numbers_buf.end(); ++iter)
			std::cout << iter->first << ' '; // выводим числа
	}
};

int main()
{
	// создаем объект хранения, задавая параметр согласно условию: кол-во чисел в ответе (1000), целевое число повторений (1000)
	number_counter counter(1000, 1000); 
	int number = 0; // временное храниние принимаемого числа

	while (std::cin >> number)
		counter.Step(number);
	
	counter.Print();

	return 0;
}