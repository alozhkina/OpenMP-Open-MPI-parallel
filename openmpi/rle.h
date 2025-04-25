#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <mpi.h>
#include "Huff.h"

using namespace std;
extern int Max;
class RLE {
	map <char, string> replace_abc;  //ключ -декод знач - код
	string encode;  //строка закод текста
	string decode;  //строка декод текста

public:
	string RLEcodes(string data);
	string get_encode(const string& name);
	string decode_text(string str);
	bool test(const string& name1, const string& name2);
	void RLEcode_parallel(const string& start, const string& end);
	void RLEdecode_parallel(const string& start, const string& end);
};

string RLE::RLEcodes(string data) {
	string result;
	int iteration = 1;
	for (int i = 0; i < data.size(); i++) {
		if (data[i] == data[i + 1]) {
			iteration++;
		}
		else {
			result += to_string(iteration);
			result.push_back(data[i]);
			result.push_back('#');
			Max = iteration > Max ? iteration : Max;
			iteration = 1;
		}
	}
	return result;
}

void RLE::RLEcode_parallel(const string& start, const string& end) {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::vector<std::string> parts;
	std::string part;
	std::string coded_part;

	if (rank == 0) {
		// Разделить файл только в процессе с рангом 0
		parts = split_file(start, size);

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

	
	coded_part = RLEcodes(part);

	
	if (rank == 0) {
		std::vector<std::string> coded_parts(size);
		coded_parts[0] = std::move(coded_part);
		for (int i = 1; i < size; i++) {
			MPI_Status status;
			MPI_Probe(i, 0, MPI_COMM_WORLD, &status);
			int count;
			MPI_Get_count(&status, MPI_CHAR, &count);
			coded_parts[i].resize(count);
			MPI_Recv(&coded_parts[i][0], count, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}// Сбор закодированных частей обратно в процесс 0

		
		std::ofstream fout(end);
		for (const auto& cp : coded_parts) {
			fout << cp << "1^#";
		}
		fout.close();
	}
	else {
		
		MPI_Send(coded_part.data(), coded_part.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD);// Отправка закодированной части обратно процессу 0
	}
//	int threads_num = omp_get_max_threads();
//	std::vector<std::string> parts = split_file(start, threads_num);
//	std::vector<std::string> coded_parts(threads_num, "");
//
//#pragma omp parallel
//	{
//		int tid = omp_get_thread_num();
//		coded_parts[tid] = RLEcodes(parts[tid]);
//	}
//
//	std::ofstream fout;
//	fout.open(end);
//
//	for (const auto& coded_part : coded_parts)
//		fout << coded_part << "1^#";
}

string RLE::get_encode(const string& name)
{
	ifstream file(name);
	string result((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	return result;
}



vector<std::string> split_file_decode1(const std::string& filename, int threads) {

	std::ifstream file(filename);

	
	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());// Считать содержимое файла в строку

	file.close();


	std::vector<std::string> parts;
	parts.reserve(threads);


	string line;

	/*char buf = '1^';*/

	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] != '^') {
			line.push_back(text[i]);
		}
		else {
			line.pop_back();
			parts.push_back(line);
			line.clear();
			++i;

		}
	}

	return parts;
}
void RLE::RLEdecode_parallel(const string& start, const string& end) {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	std::vector<std::string> parts;
	std::string part;
	std::string decoded_part;

	if (rank == 0) {
		
		std::vector<std::string> parts = split_file_decode1(start, size);

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

	decoded_part = decode_text(part);

	// Сбор декодированных частей обратно в процесс 0
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
		}

	
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
	std::vector<std::string> parts = split_file_decode1(start, threads_num);
	std::vector<std::string> coded_parts(threads_num, "");

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		coded_parts[tid] = decode_text(parts[tid]);
	}

	std::ofstream fout;
	fout.open(end);

	for (const auto& coded_part : coded_parts)
		fout << coded_part;*/
}





string RLE::decode_text(string str) {
	string decode;
	string p;
	for (int i = 0; i < str.size(); i++) {
		//int c = 0;

		if (str[i] != '#') {
			/*	c++;*/
			p.push_back(str[i]);
			/*int y = str[i-2] - '0';
			decode.append(y,str[i-1]);*/
		}
		else if (p.size() > 0) {
			p.pop_back();
			int y{ stoi(p) };
			decode.append(y, str[i - 1]);
			Max = y > Max ? y : Max;
			p.erase();
		}



	}
	return decode;
	/*string text;
	fstream input;
	input.open(str, ios::in);
	while (!input.eof())
	{
		string temp;
		getline(input, temp);
		if (!'#')
		{
			temp += '\n';
		}
		text += temp;
	}
	input.close();
	return text;*/

	//string p;
	//for (int i = 0; i < str.size(); i+=2) {
	//	p.push_back(str[i]);
	//	int y{stoi(p)};
	//	p.pop_back();
	//	decode.append(y, str[i+1]);
	//	Max = y > Max ? y : Max;
	//}
	//return decode;
}

bool RLE::test(const string& name1, const string& name2) {
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