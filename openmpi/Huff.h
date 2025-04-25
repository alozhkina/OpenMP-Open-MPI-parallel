#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <mpi.h>

using namespace std;


class huffman_tree_node
{
private:
	char data;									// Символ
	unsigned freq;								// Частота
	huffman_tree_node* left, * right;			// Левый и правый потомок
	friend class huffman_coding;
public:
	huffman_tree_node() {};
	huffman_tree_node(char _data, unsigned _freq);			// Создание узла
};

typedef huffman_tree_node* pointer_huffman;
typedef huffman_tree_node** p_pointer_huffman;

inline huffman_tree_node::huffman_tree_node(char _data, unsigned _freq)
{
	left = right = NULL;
	data = _data;
	freq = _freq;
}

class huffman_coding
{
private:
	pointer_huffman root_main;
	map <char, string> replace_abc;  //ключ -декод знач - код
	string encode;  //строка закод текста
	string decode;  //строка декод текста
public:
	struct compare
	{
		bool operator()(huffman_tree_node* left, huffman_tree_node* right)
		{
			return (left->freq > right->freq); // 0 и 1
		}
	};
	void printCodes(huffman_tree_node* root, string str);						// Печать кодов, создание map!!!
	void HuffmanCodes(string data, double freq[], bool flag_ex);					// Кодирование
	string encode_text(string text);												// Кодирование-замена в самом стринге
	string get_encode(const string& name);										// Вывод в файл

	string decode_text(string str, bool flag_ex);	
	bool test(const string& name1, const string& name2);
	void parallel_huf_encode_text(const string& start, const string& end);

	void parallel_huf_decode_text(const string& start, const string& end, bool flag_ex);
};

string huffman_coding::get_encode(const string& name)
{
	ifstream file(name);
	string result((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	return result;
}

/* Вывод текста в файл*/


//возвращаемся по дереву, вызывая рекурсию, пример: дошли до начала по левому краю, у него проверили лево и право (по нулям),
// дальше вернулись на один и пошли записали справа, и так каждый раз возвращаемся.
void huffman_coding::printCodes(huffman_tree_node* root, string str)
{

	if (!root)
	{
		return;
	}
	if (root->data != '$')
	{
	/*	cout << root->data << ": " << str << endl;*/
		replace_abc.insert(pair <char, string>(root->data, str));

	}

	printCodes(root->left, str + "0");
	printCodes(root->right, str + "1");

}

void huffman_coding::HuffmanCodes(string data, double freq[], bool flag_ex)
{

	pointer_huffman left, right, top; //это те же указатели

	// Создаем узлы с данными и частотой (учитывая compare функцию сравнения)
	priority_queue<pointer_huffman, vector<huffman_tree_node*>, compare> min_heap;  //очередь с приоритетами
	for (int i = 0; i < data.size(); ++i)
	{

		pointer_huffman temp = new huffman_tree_node(data[i], freq[i]);  //определяю свой указатель
		
		min_heap.push(temp); //добавляю в очередь
	}
	// Пока размер очереди не дойдёт до единицы
	while (min_heap.size() != 1)
	{
		// Достаем два минимальных по частоте элемента из дерева
		left = min_heap.top();
		min_heap.pop();

		right = min_heap.top();
		min_heap.pop();

		// Создаем новый узел с частотой, равной сумме двух 
		// частот узлов c наименьшей частотой

		top = new huffman_tree_node('$', left->freq + right->freq);
		top->left = left;
		top->right = right;
		min_heap.push(top);
	}

	// Выводим на экран код Хаффмана
	root_main = min_heap.top();
	if (flag_ex)
	{
		printCodes(root_main, "0");
	}
	else
	{
		printCodes(root_main, "");
	}
}
vector<std::string> split_file(const std::string& filename, int threads) {

	std::ifstream file(filename);


	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());	// Считать содержимое файла в строку


	file.close();

	// Вычислить размер одной части
	size_t part_size = text.size() / threads;
	std::vector<std::string> parts;
	parts.reserve(threads);

	
	for (size_t i = 0; i < threads; ++i) {
		size_t start = i * part_size;
		size_t end = (i == threads - 1) ? text.size() : (i + 1) * part_size;
		parts.push_back(text.substr(start, end - start));
	}// Разделить содержимое на части

	return parts;
}
vector<std::string> split_file_decode(const std::string& filename, int threads) {
	
	std::ifstream file(filename);

	// Считать содержимое файла в строку
	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());


	file.close();


	std::vector<std::string> parts;
	parts.reserve(threads);


	string line;
	
	char buf = '@';

	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] != buf) {
			line.push_back(text[i]);
		}
		else {
			parts.push_back(line);
			line.clear();
		}
	}// Разделить содержимое на части

	return parts;
}
//------------------------------------------------------------------------------------------------------------------


void huffman_coding::parallel_huf_encode_text(const string& start, const string& end)
{
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::vector<std::string> parts;
	std::string part;
	std::string coded_part;

	if (rank == 0) {// Разделить файл только в процессе с рангом 0
		
		parts = split_file(start, size);

		
		for (int i = 1; i < size; i++) {
			MPI_Send(parts[i].data(), parts[i].size(), MPI_CHAR, i, 0, MPI_COMM_WORLD);// Рассылка каждой части файла соответствующему процессу
		}
		part = parts[0];  // Часть для процесса 0
	}
	else {
		MPI_Status status;
		MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
		int count;
		MPI_Get_count(&status, MPI_CHAR, &count);
		part.resize(count);
		MPI_Recv(&part[0], count, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	
	coded_part = encode_text(part);

	
	if (rank == 0) {
		std::vector<std::string> coded_parts(size);
		coded_parts[0] = std::move(coded_part);
		for (int i = 1; i < size; i++) {
			MPI_Status status;
			MPI_Probe(i, 0, MPI_COMM_WORLD, &status);
			int count;
			MPI_Get_count(&status, MPI_CHAR, &count);
			coded_parts[i].resize(count);
			MPI_Recv(&coded_parts[i][0], count, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);// Сбор закодированных частей обратно в процесс 0
		}

		
		std::ofstream fout(end);
		for (const auto& cp : coded_parts) {
			fout << cp << '@';
		}
		fout.close();
	}
	else {
		
		MPI_Send(coded_part.data(), coded_part.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD);// Отправка закодированной части обратно процессу 0
	}

//
//	int threads_num = omp_get_max_threads();
//	std::vector<std::string> parts = split_file(start, threads_num);
//	std::vector<std::string> coded_parts(threads_num, "");
//
//#pragma omp parallel
//	{
//
//		int tid = omp_get_thread_num();
//		coded_parts[tid] = encode_text(parts[tid]);
//	}
//
//	std::ofstream fout;
//	fout.open(end);
//
//	for (const auto& coded_part : coded_parts)
//		fout << coded_part << '@';

}

string huffman_coding::encode_text(string text) {
	string result;
	for (int i = 0; i < text.size(); i++)
	{
		for (auto it = replace_abc.begin(); it != replace_abc.end(); ++it)
		{
			if (text[i] == it->first)
			{
				result.append(it->second);
			}
		}
	}
	return result;
}

void huffman_coding::parallel_huf_decode_text(const string& start, const string& end, bool flag_ex) {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::vector<std::string> parts;
	std::string part;
	std::string decoded_part;

	if (rank == 0) {
		// Разделить файл только в процессе с рангом 0
		std::vector<std::string> parts = split_file_decode(start, size);

		// Рассылка каждой части файла соответствующему процессу
		for (int i = 1; i < size; i++) {
			MPI_Send(parts[i].data(), parts[i].size(), MPI_CHAR, i, 0, MPI_COMM_WORLD);
		}
		part = parts[0];  // Часть для процесса 0
	}
	else {
		MPI_Status status;
		MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
		int count;
		MPI_Get_count(&status, MPI_CHAR, &count);
		part.resize(count);
		MPI_Recv(&part[0], count, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	decoded_part = decode_text(part, flag_ex);

	
	if (rank == 0) {
		std::vector<std::string> decoded_parts(size);
		decoded_parts[0] = std::move(decoded_part);
		for (int i = 1; i < size; i++) {
			MPI_Status status;
			MPI_Probe(i, 0, MPI_COMM_WORLD, &status);
			int count;
			MPI_Get_count(&status, MPI_CHAR, &count);
			decoded_parts[i].resize(count);
			MPI_Recv(&decoded_parts[i][0], count, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}// Сбор декодированных частей обратно в процесс 0

		
		std::ofstream fout(end);
		for (const auto& dp : decoded_parts) {
			fout << dp;
		}
		fout.close();
	}
	else {
		
		MPI_Send(decoded_part.data(), decoded_part.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD);// Отправка закодированной части обратно процессу 0
	}
	/*int threads_num = omp_get_max_threads();
	std::vector<std::string> parts = split_file_decode(start, threads_num);
	std::vector<std::string> coded_parts(threads_num, "");

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		coded_parts[tid] = decode_text(parts[tid], flag_ex);
	}

	std::ofstream fout;
	fout.open(end);

	for (const auto& coded_part : coded_parts)
		fout << coded_part;*/
}
/* Раскодирование текста */
//опускаемся по дереву, пока не дойдем до символа
string huffman_coding::decode_text(string str, bool flag_ex)
{
	int main_counter = 0;
	string decode = "";
	for (int i = main_counter; i < str.size();)
	{
		pointer_huffman temp = root_main;
		while (true)
		{
			if (i < str.size() && str[i] == '0')
			{
				if (temp->left != NULL)
				{
					temp = temp->left;
				}
				else
				{
					decode += temp->data;
					break;
				}
			}
			else
			{
				if (temp->right != NULL)
				{
					temp = temp->right;
				}
				else
				{
					decode += temp->data;
					break;
				}
			}
			i++;
			main_counter = i;
		}


	}
	return decode;
}







bool huffman_coding::test(const string& name1, const string& name2) {
	ifstream filepr(name1);
	ifstream filedec(name2);
	string str((std::istreambuf_iterator<char>(filepr)),
		std::istreambuf_iterator<char>());
	string dec((std::istreambuf_iterator<char>(filedec)),
		std::istreambuf_iterator<char>());
	if (str.size() != dec.size()) {
		return 0;
	}
	bool flag = false;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] != dec[i]) {

			return false;
		}
	}
	return true;
}

void generate(string init_dict, const std::string& file_name) {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int alphabet_size = init_dict.size();
	int local_size = 10000 / size;  // размер задачи на процесс
	std::string local_buffer;

	if (rank == 0) {
		local_size += 10000 % size;
	}

	// Генерация данных в каждом процессе
	srand(time(NULL) + rank);
	for (int i = 0; i < local_size; ++i) {
		local_buffer += init_dict[rand() % alphabet_size];
	}

	// Сбор закодированных частей в процесс 0
	if (rank == 0) {
		std::vector<std::string> generated_parts(size);
		generated_parts[0] = std::move(local_buffer);
		for (int i = 1; i < size; i++) {
			MPI_Status status;
			MPI_Probe(i, 0, MPI_COMM_WORLD, &status);
			int count;
			MPI_Get_count(&status, MPI_CHAR, &count);
			generated_parts[i].resize(count);
			MPI_Recv(&generated_parts[i][0], count, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// Процесс 0 записывает собранные данные в файл
		std::ofstream file(file_name);
		for (const auto& gp : generated_parts) {
			file << gp;
		}
		file.close();
	}
	else {
		// Отправка сгенерированной части процессу 0
		MPI_Send(local_buffer.data(), local_buffer.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
}