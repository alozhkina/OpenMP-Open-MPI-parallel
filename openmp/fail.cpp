#define _CRT_SECURE_NO_WARNINGS

#define filename_input "prpr.txt"
#define filename_input2 "encode.txt"
#define filename_input3 "decode.txt"
#define filename_input4 "encRLE.txt"
#define filename_input5 "decRLE.txt"
#define filename_input6 "encRLEHUFF.txt"
#define filename_input7 "decRLEHUFF.txt"
#define filename_input8 "encHUFF+.txt"
#define filename_input9 "decHUFF+.txt"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "coding.h"
#include "RLE.h"
#include <omp.h>

#include <string>

#include <fstream>
#include <queue>
#include <map>


using namespace std;
const char Mass[] = { 'ф', 'х', 'ц', 'ч', 'ш', 'у', 'У', 'Ф', 'Х', 'Ц', 'Ч', 'Ш', '1', '2', '3', '4', '5', '6','7', '8', '9', '0', '.', '&', ' ' };
int Max;
string get_data(string filename)
{
	string text;
	fstream input;
	input.open(filename, ios::in);
	while (!input.eof())
	{
		string temp;
		getline(input, temp);
		if (!input.eof())
		{
			temp += '\n';
		}
		text += temp;
	}
	input.close();
	return text;
}

void get_frequency(string text, double* frequencies, string Mass)
{
	for (int i = 0; i < Mass.size(); i++)
	{
		frequencies[i] = 0;
	}
#pragma omp parallel for 
	for (int j = 0; j < Mass.size(); j++)
	{
		for (int i = 0; i < text.size(); i++)
		{
			if (Mass[j] == text[i])
			{
				frequencies[j]++;
			}
		}
	}


}

//const char* MASS[] = {"ф", "х", "ц", "ч", "ш", "Ф", "Х", "Ц", "Ч", "Ш", "у", "У", "&", ".", " ", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
int main()
{
	setlocale(LC_ALL, "rus");
	/* FILE* pFile;
	 pFile = fopen("prpr.txt", "w");*/
	ofstream file1("prpr.txt");
	int val = 10000;
	int i;
	srand(time(0));
	int index;
	string p;
	std::vector<std::string> local_buffers(omp_get_max_threads());//количество потоков

#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();//номер потока
		std::string& buffer = local_buffers[thread_id];
		int local_size = val / omp_get_num_threads();//сколько потоков в моменте
		if (thread_id == 0) {
			local_size += val % omp_get_num_threads();
		}
		for (int i = 0; i < local_size; i++) {
			buffer += Mass[rand() % (sizeof(Mass) / sizeof(*Mass))];
		}

	}
	for (const std::string& buffer : local_buffers) {
		file1 << buffer;
	}
	/*   for (i = 0; i < val; ++i)
	   {
		   int h = (sizeof(Mass) / sizeof(*Mass));
		   index = rand() % (sizeof(Mass) / sizeof(*Mass));
		   p.push_back(Mass[index]);

	   }*/
	   /*file1<<p;*/
	file1.close();/*
	fclose(pFile);*/


	string text;
	text = get_data(filename_input);

	string MASS1;
	MASS1.append(Mass);
	double* frequencies = new double[MASS1.size()];
	get_frequency(text, frequencies, MASS1);
	int c = 0;
	cout << "  Huffman: " << endl << endl;
	cout << "Frequencies: " << endl;
	for (int i = 0; i < MASS1.size(); i++)
	{
		cout << MASS1[i] << " - " << frequencies[i] << endl;
		c += frequencies[i];

	}
	cout << endl;

	/* Обработка исключения: один символ */
	bool flag_ex = 0;
	if (MASS1.size() < 2)
	{
		flag_ex = !flag_ex;
	}
	huffman_coding encoding;
	cout << endl << "Character: Code-word" << endl;
	encoding.HuffmanCodes(MASS1, frequencies, flag_ex);
	encoding.parallel_huf_encode_text("prpr.txt", "encode.txt");

	/*string temp = encoding.get_encode();*/

	//cout << endl << "Code-string: " << temp;

	/*encoding.output_file(filename_input2, true);*/

	encoding.parallel_huf_decode_text("encode.txt", "decode.txt", flag_ex);
	if (MASS1.size() < 2)
	{
		cout << endl << "Decode-string: " << text << endl;
	}


	double price_cod;
	double ratio = encoding.compression_ratio("encode.txt");
	price_cod = encoding.price(MASS1, frequencies);

	cout << endl << "Price coding: " << price_cod << endl;
	cout << endl << "Сompression_ratio: " << ratio << endl;
	if (encoding.test("prpr.txt", "decode.txt") == 1) {
		cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
	}
	else cout << endl << "TEST: decoding is not correct" << endl;

	delete[] frequencies;

	//---------------------------------------------------------------------------------------------------------------  RLE
	cout << "  RLE: " << endl;
	RLE rabbit;
	rabbit.RLEcode_parallel("prpr.txt", "encRLE.txt");/*
	string temp2 = rabbit.get_encode();*/
	/*cout << endl << "Code-string: " << rabbit.get_encode();*/
	/*rabbit.output_file(filename_input4, true);*/
	double ratio2 = rabbit.compression_ratio("encRLE.txt");
	cout << endl << "Сompression_ratio: " << ratio2 << endl;


	rabbit.RLEdecode_parallel("encRLE.txt", "decRLE.txt");
	//rabbit.output_file(filename_input5, false);
	if (rabbit.test("prpr.txt", "decRLE.txt") == 1) {
		cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
	}
	else cout << endl << "TEST: decoding is not correct" << endl;

	//-------------------------------------------------------------------------------------------------------------- RLE + Huffman
	cout << "  RLE+Huffman: " << endl<<endl;
	RLE wolf;
	huffman_coding fox;/*
	wolf.RLEcode_parallel(text);*/
	string temp3 = rabbit.get_encode("encRLE.txt");
	int k = 10;
	string MASS;
	MASS.append(Mass);
	MASS.append("^");
	MASS.append("#");
	if (Max > 9) {
		for (int i = 9; i < Max; i++) {
			MASS += to_string(k);
			k++;
		}
	}
	double* chance = new double[MASS.size()];
	get_frequency(temp3, chance, MASS);
	cout << "c)  Frequencies: " << endl;
	for (int i = 0; i < MASS.size(); i++)
	{
		cout << MASS[i] << " - " << chance[i] << endl;
	}
	cout << endl;

	/* Обработка исключения: один символ */
	bool flag_ex2 = 0;
	if (MASS.size() < 2)
	{
		flag_ex2 = !flag_ex2;
	}

	fox.HuffmanCodes(MASS, chance, flag_ex2);
	fox.parallel_huf_encode_text("encRLE.txt", "encRLEHUFF.txt");

	/*string temp4 = fox.get_encode();*/

	//fox.output_file(filename_input6, true);

	double ratio3 = fox.compression_ratio("encRLEHUFF.txt", chance, "encRLE.txt");
	cout << endl << "Сompression_ratio: " << ratio3 << endl;

	fox.parallel_huf_decode_text("encRLEHUFF.txt", "decRLEHUFF1.txt", flag_ex2);

	//if (wolf.test("encRLE.txt", "decRLEHUFF1.txt") == 1) {
	//	cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
	//}
	//else cout << endl << "TEST: decoding is not correct" << endl;

    wolf.RLEdecode_parallel("decRLEHUFF1.txt", "decRLEHUFF.txt");
//
//	wolf.output_file(filename_input7, false);
//
	if (wolf.test("prpr.txt", "decRLEHUFF.txt") == 1) {
		cout << endl << "TEST: decoding is correct" << endl<<"----------------------------------------------------------------------"<<endl << endl;
	}
	else cout << endl << "TEST: decoding is not correct" << endl;

	delete[] chance;

//	//--------------------------------------------------------------------------------------------------------------  Huffman + RLE
//
	cout << "  Huffman+RLE: " << endl<<endl;
	RLE fish;
	huffman_coding bear;
	string MASS2;
	MASS2.append(Mass);
	double* chance2 = new double[MASS2.size()];
	get_frequency(text, chance2, MASS2);
	cout << "Frequencies: " << endl;
	for (int i = 0; i < (MASS2.size()); i++)
	{
		cout << MASS2[i] << " - " << chance2[i] << endl;

	}
	cout << endl;
	bool flag_ex5 = 0;
	if (MASS2.size() < 2)
	{
		flag_ex5 = !flag_ex5;
	}
	bear.HuffmanCodes(MASS2, chance2, flag_ex5);
	//bear.parallel_huf_encode_text();
//
//	string buf = bear.get_encode();
//
//
//	//+
	fish.RLEcode_parallel("encode.txt", "encHUFF+.txt");
//	string buf2 = fish.get_encode();
//	/*cout << endl << "Code-string: " << rabbit.get_encode();*/
//	fish.output_file(filename_input8, true);flag
	double ratio4 = fish.compression_ratio("encHUFF+.txt");
	cout << endl << "Сompression_ratio: " << ratio4<< endl;
//
    fish.RLEdecode_parallel("encHUFF+.txt", "decHUFF+1.txt");
//
//
//
//
     bear.parallel_huf_decode_text("decHUFF+1.txt", "decHUFF+.txt", flag_ex5);
//    bear.output_file(filename_input9, false);
	if (bear.test("prpr.txt", "decHUFF+.txt") == 1) {
		cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------"<<endl<<endl;
	}
	else cout << endl << "TEST: decoding is not correct" << endl;
//
	delete[] chance2;
	Max = 0;

	if (ratio >= ratio2 && ratio >= ratio3 && ratio >= ratio4) cout << "Huffman algorithm is better."<<endl;
	if (ratio2 >= ratio && ratio2 >= ratio3 && ratio2 >= ratio4) cout << "RLE algorithm is better."<<endl;
	if (ratio3 >= ratio && ratio3 >= ratio2 && ratio3 >= ratio4) cout << "RLE+Huffman algorithms are better."<<endl;
	if (ratio4 >= ratio && ratio4 >= ratio2 && ratio4 >= ratio3) cout << "Huffman+RLE algorithms are better."<<endl;

}
//#include <fstream>
//using namespace std;
//
//int main(int argc, char* argv[])
//{
//    ofstream fout("prpr.txt"); // создаём объект класса ofstream для записи и связываем его с файлом cppstudio.txt
//    fout << "Работа с файлами в С++"; // запись строки в файл
//    fout.close(); // закрываем файл
//    system("pause");
//    return 0;
//}
