#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <omp.h>
#include "coding.h"

using namespace std;
extern int Max;
class RLE{
	map <char, string> replace_abc;  //ключ -декод знач - код
	string encode;  //строка закод текста
	string decode;  //строка декод текста

public:
	string RLEcodes(string data);
	string get_encode(const string& name);
	void output_file(string filename, bool flag);
	double compression_ratio(const string& name);
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
			result+= to_string(iteration);
			result.push_back(data[i]);
			result.push_back('#');
			Max = iteration > Max ? iteration : Max;
			iteration = 1;
		}
	}
	return result;
}

void RLE::RLEcode_parallel(const string& start, const string& end) {
	int threads_num = omp_get_max_threads();
	std::vector<std::string> parts = split_file(start, threads_num);
	std::vector<std::string> coded_parts(threads_num, "");

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		coded_parts[tid] = RLEcodes(parts[tid]);
	}

	std::ofstream fout;
	fout.open(end);

	for (const auto& coded_part : coded_parts)
		fout << coded_part << "1^#";
}

string RLE::get_encode(const string& name)
{
	ifstream file(name);
	string result((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	return result;
}

void RLE::output_file(string filename, bool flag)
{
	fstream file;
	file.open(filename, ios::out);
	if (flag)
	{
		for (int i = 0; i < encode.size(); i++)
		{
			if (encode[i] != '#') {
				file << encode[i];
			}
		}
	}
	else
	{
		for (int i = 0; i < decode.size(); i++)
		{
			file << decode[i];
		}
	}

	file.close();
}
vector<std::string> split_file_decode1(const std::string& filename, int threads) {
	// Открыть файл
	std::ifstream file(filename);

	// Считать содержимое файла в строку
	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	// Закрыть файл
	file.close();


	std::vector<std::string> parts;
	parts.reserve(threads);


	string line;
	// Разделить содержимое на части
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
	int threads_num = omp_get_max_threads();
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
		fout << coded_part;
}
double RLE::compression_ratio(const string& name) {
	ifstream file(name);
	string enc((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
	
	int c = 0;
/*#pragma omp parallel for  */                              ///!!!!!!!!!!!!!
	for (int i = 0; i <= enc.size(); i++) {
		if (enc[i] == '#' || enc[i] =='^') {
			c++;
		}
	}
	double tmp = (10000. * 8) / ((enc.size()-c)*8);
	return tmp;

}



string RLE::decode_text(string str) {
	string decode;
	string p;
	for (int i = 0; i < str.size(); i++) {
		//int c = 0;
		
		if (str[i]!= '#' ) {
		/*	c++;*/
			p.push_back(str[i]);
			/*int y = str[i-2] - '0';
			decode.append(y,str[i-1]);*/
		}
		else if (p.size()>0) {
			p.pop_back();
			int y{ stoi(p) };
			decode.append(y, str[i-1]);
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