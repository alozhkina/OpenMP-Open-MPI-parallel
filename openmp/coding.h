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
#include <omp.h>

using namespace std;


class huffman_tree_node
{
private:
	char data;									// ������
	unsigned freq;								// �������
	huffman_tree_node* left, * right;			// ����� � ������ �������
	friend class huffman_coding;
public:
	huffman_tree_node() {};
	huffman_tree_node(char _data, unsigned _freq);			// �������� ����
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
	map <char, string> replace_abc;  //���� -����� ���� - ���
	string encode;  //������ ����� ������
	string decode;  //������ ����� ������
public:
	struct compare
	{
		bool operator()(huffman_tree_node* left, huffman_tree_node* right)
		{
			return (left->freq > right->freq); // 0 � 1
		}
	};
	void printCodes(huffman_tree_node* root, string str);						// ������ �����, �������� map!!!
	void HuffmanCodes(string data, double freq[], bool flag_ex);					// �����������
	string encode_text(string text);												// �����������-������ � ����� �������
	string get_encode(const string& name);												// ������� encode ������
	void output_file(string filename, bool flag);								// ����� � ����
	
	string decode_text(string str, bool flag_ex);								// �������������� ����� ������
	double price(string data, double* frequency);                                 //���� �����������
	double compression_ratio(const string& name, double* frequencies, const string& name2);
	double compression_ratio(const string& name);
	bool test(const string& name1, const string& name2);

	string decode_textRLE(string str, bool flag_ex);
	void parallel_huf_encode_text(const string& start, const string& end);

	void parallel_huf_decode_text(const string& start, const string& end, bool flag_ex);
	void parallel_huf_decode_textRLE(const string& start, const string& end, bool flag_ex);
};

string huffman_coding::get_encode(const string& name)
{
	ifstream file(name);
	string result((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	return result;
}

/* ����� ������ � ����*/
void huffman_coding::output_file(string filename, bool flag)
{
	fstream file;
	file.open(filename, ios::out);
	if (flag)
	{
		for (int i = 0; i < encode.size(); i++)
		{
			file << encode[i];
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

//������������ �� ������, ������� ��������, ������: ����� �� ������ �� ������ ����, � ���� ��������� ���� � ����� (�� �����),
// ������ ��������� �� ���� � ����� �������� ������, � ��� ������ ��� ������������.
void huffman_coding::printCodes(huffman_tree_node* root, string str)
{

	if (!root)
	{
	   return;
	}
	if (root->data != '$')
	{
		cout << root->data << ": " << str << endl;
		replace_abc.insert(pair <char, string>(root->data, str));
		
	}

	printCodes(root->left, str + "0");
	printCodes(root->right, str + "1");

}

void huffman_coding::HuffmanCodes(string data, double freq[], bool flag_ex)
{
	pointer_huffman left, right, top; //��� �� �� ���������
	
	// ������� ���� � ������� � �������� (�������� compare ������� ���������)
	priority_queue<pointer_huffman, vector<huffman_tree_node*>, compare> min_heap;  //������� � ������������
	for (int i = 0; i < data.size(); ++i)
	{
		
		pointer_huffman temp = new huffman_tree_node(data[i], freq[i]);  //��������� ���� ���������
		cout << "Hello" << endl;
		min_heap.push(temp); //�������� � �������
	}
	// ���� ������ ������� �� ����� �� �������
	while (min_heap.size() != 1)
	{
		// ������� ��� ����������� �� ������� �������� �� ������
		left = min_heap.top();
		min_heap.pop();

		right = min_heap.top();
		min_heap.pop();

		// ������� ����� ���� � ��������, ������ ����� ���� 
		// ������ ����� c ���������� ��������

		top = new huffman_tree_node('$', left->freq + right->freq);
		top->left = left;
		top->right = right;
		min_heap.push(top);
	}

	// ������� �� ����� ��� ��������
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
	// ������� ����
	std::ifstream file(filename);

	// ������� ���������� ����� � ������
	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	// ������� ����
	file.close();

	// ��������� ������ ����� �����
	size_t part_size = text.size() / threads;
	std::vector<std::string> parts;
	parts.reserve(threads);

	// ��������� ���������� �� �����
	for (size_t i = 0; i < threads; ++i) {
		size_t start = i * part_size;
		size_t end = (i == threads - 1) ? text.size() : (i + 1) * part_size;
		parts.push_back(text.substr(start, end - start));
	}

	return parts;
}
vector<std::string> split_file_decode(const std::string& filename, int threads) {
	// ������� ����
	std::ifstream file(filename);

	// ������� ���������� ����� � ������
	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	// ������� ����
	file.close();


	std::vector<std::string> parts;
	parts.reserve(threads);


	string line;
	// ��������� ���������� �� �����
	char buf = '@';

	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] != buf ) {
			line.push_back(text[i]);
		}
		else {
			parts.push_back(line);
			line.clear();
		}
	}

	return parts;
}
//------------------------------------------------------------------------------------------------------------------


void huffman_coding::parallel_huf_encode_text(const string& start, const string& end)
{

	int threads_num = omp_get_max_threads();
	std::vector<std::string> parts = split_file(start, threads_num);
	std::vector<std::string> coded_parts(threads_num, "");

#pragma omp parallel
	{

		int tid = omp_get_thread_num();
		coded_parts[tid] = encode_text(parts[tid]);
	}

	std::ofstream fout;
	fout.open(end);

	for (const auto& coded_part : coded_parts)
		fout << coded_part<<'@';

}

string huffman_coding::encode_text(string text) {
	string result = "";
	for (int i = 0; i < text.size(); i++)
	{
		for (auto it = replace_abc.begin(); it != replace_abc.end(); ++it)
		{
			if ( text[i] == it->first)
			{
				result.append(it->second);
			}
		}
	}
	return result;
}

void huffman_coding::parallel_huf_decode_text(const string& start, const string& end, bool flag_ex) {
	int threads_num = omp_get_max_threads();
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
		fout << coded_part;
}
/* �������������� ������ */
//���������� �� ������, ���� �� ������ �� �������
string huffman_coding::decode_text(string str, bool flag_ex)
{
	int main_counter = 0;
	string decode = "";
	for (int i = main_counter; i < str.size();)
	{
		pointer_huffman temp = root_main;
		while (true)
		{
			if (i<str.size() && str[i] == '0')
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

void huffman_coding::parallel_huf_decode_textRLE(const string& start, const string& end, bool flag_ex) {
	int threads_num = omp_get_max_threads();
	std::vector<std::string> parts = split_file_decode(start, threads_num);
	std::vector<std::string> coded_parts(threads_num, "");

#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		coded_parts[tid] = decode_textRLE(parts[tid], flag_ex);
	}

	std::ofstream fout;
	fout.open(end);

	for (const auto& coded_part : coded_parts)
		fout << coded_part;
}

string huffman_coding::decode_textRLE(string str, bool flag_ex)
{
	int main_counter = 0;
	string decode1 = "";
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
					decode1 += temp->data;
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
					decode1 += temp->data;
					break;
				}
			}
			i++;
			main_counter = i;
		}

	/*	if (flag_ex)
		{
			i++;
		}
		else
		{
			i = main_counter;
		}*/
	}
	decode = decode1;
	for (int i = 2; i<=decode.size();i+=3)
	{
		decode.insert(i, "#");
	}
	return decode;
}


double huffman_coding::price(string data, double* frequencies) {
	double price_coding = 0;
	for (int i = 0; i < data.size(); i++)
	{
		for (auto it = replace_abc.begin(); it != replace_abc.end(); ++it)
		{
			if (data[i] == it->first)
			{
				price_coding += (it->second.size()) * (frequencies[i] / 10000);
			}
		}
	}
	return price_coding;
	/*int i = 0;
	for (auto it = replace_abc.begin(); it != replace_abc.end(); ++it)
	{
		if (data[i] == it->first)
		{
			encode += it->second;
		}
		price_coding += (it->second.size()) * (frequencies[i] / 10000);
		i++;
	}*/
}
double huffman_coding::compression_ratio(const string& name) {
	ifstream file(name);
	string enc;
	file >> enc;
	int c = 0;
	for (int i = 0; i <= enc.size(); i++) {
		if (enc[i] == '#') {
			c++;
		}
	}
	double tmp = 10000. * 8 / (enc.size() - c);
	return tmp;

}
double huffman_coding::compression_ratio(const string& name, double* chance, const string& name2 ) {
	ifstream file(name);
	string enc;
	file >> enc;
	double tmp3 = (chance[26] * 3);
	double tmp2 = enc.size() - tmp3;
	double tmp = 10000.*8 / tmp2;
	return tmp;

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