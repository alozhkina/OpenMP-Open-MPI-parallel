#define _CRT_SECURE_NO_WARNINGS

#define filename_input "prpr2.txt"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "Huff.h"
#include "rle.h"
#include <mpi.h>

#include <string>

#include <fstream>
#include <queue>
#include <map>


using namespace std;
const char Mass[] = { 'ф', 'х', 'ц', 'ч', 'ш', 'у', 'У', 'Ф', 'Х', 'Ц', 'Ч', 'Ш', '1', '2', '3', '4', '5', '6','7', '8', '9', '0', '.', '&', ' ' };
int Max;
//string get_data(string filename)
//{
//	string text;
//	fstream input;
//	input.open(filename, ios::in);
//	while (!input.eof())
//	{
//		string temp;
//		getline(input, temp);
//		if (!input.eof())
//		{
//			temp += '\n';
//		}
//		text += temp;
//	}
//	input.close();
//	return text;
//}

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
	MPI_Init(nullptr, nullptr);
	string MASS1;
	
	
	setlocale(LC_ALL, "rus");

	/* FILE* pFile;
	 pFile = fopen("prpr.txt", "w");*/
	 //ofstream file1("prpr2.txt");

	MASS1.append(Mass);
	int val = 10000;

	std::string file_name = "prpr2.txt";
	generate(MASS1, file_name);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	/*srand(time(0));*/
	//int rank, size;
	//MPI_Comm_rank(MPI_COMM_WORLD, &rank);//номер потока
	//MPI_Comm_size(MPI_COMM_WORLD, &size);//кол-во потоков

	//int alphabet_size = MASS1.size();
	//int local_size = val / size;  // размер задачи на процесс
	//std::string local_buffer;

	//if (rank == 0) {
	//	local_size += val % size;//если не лошли до 10к
	//}

	//// Генерация данных в каждом процессе
	//srand(time(NULL) + rank);
	//for (int i = 0; i < local_size; ++i) {
	//	local_buffer += MASS1[rand() % alphabet_size];
	//}

	//// Сбор закодированных частей в процесс 0
	//if (rank == 0) {
	//	std::vector<std::string> generated_parts(size);
	//	generated_parts[0] = std::move(local_buffer);//конструктор перемещения
	//	for (int i = 1; i < size; i++) {
	//		MPI_Status status;//есть ли сообщения 
	//		MPI_Probe(i, 0, MPI_COMM_WORLD, &status);//просмотр сообщения, без удаления из буфера сообщений (есть ли сообщения от итого потока)
	//		int count;
	//		MPI_Get_count(&status, MPI_CHAR, &count);//если есть, то считает размер сообщения
	//		generated_parts[i].resize(count);//не знали размер
	//		MPI_Recv(&generated_parts[i][0], count, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//принять 
	//	}

	//	// Процесс 0 записывает собранные данные в файл
	//	std::ofstream file("prpr2.txt");
	//	for (const auto& gp : generated_parts) {
	//		file << gp;
	//	}
	//	file.close();
	//}
	//else {
	//	// Отправка сгенерированной части процессу 0
	//	MPI_Send(local_buffer.data(), local_buffer.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD);//отправленный тип данных, номер потока, которому отправляем, тег, коммутатор
	//}
	/*   for (i = 0; i < val; ++i)
	   {
		   int h = (sizeof(Mass) / sizeof(*Mass));
		   index = rand() % (sizeof(Mass) / sizeof(*Mass));
		   p.push_back(Mass[index]);

	   }*/
	   /*file1<<p;*/
/*	file1.close();*//*
	fclose(pFile);*/

	ifstream filepr("prpr2.txt");
	string text((std::istreambuf_iterator<char>(filepr)),
		std::istreambuf_iterator<char>());
	/*cout << "  Huffman0: " << endl << endl;*/
	double* frequencies = new double[MASS1.size()];
	huffman_coding encoding;
	bool flag_ex = 0;
		/*text = get_data(filename_input);*/
		/*string MASS1;
		MASS1.append(Mass);*/
		/*double* frequencies = new double[MASS1.size()];*/
		get_frequency(text, frequencies, MASS1);
		int c = 0;
		if (rank == 0) {
			cout << "  Huffman: " << endl << endl;
		}
		/*cout << "Frequencies: " << endl;*/
	/*	for (int i = 0; i < MASS1.size(); i++)
		{
			cout << MASS1[i] << " - " << frequencies[i] << endl;
			c += frequencies[i];

		}*/
		cout << endl;

		/* Обработка исключения: один символ */
	/*	bool flag_ex = 0;*/
		if (MASS1.size() < 2)
		{
			flag_ex = !flag_ex;
		}
		//huffman_coding encoding;
		/*cout << endl << "Character: Code-word" << endl;*/
		encoding.HuffmanCodes(MASS1, frequencies, flag_ex);
	
	encoding.parallel_huf_encode_text("prpr2.txt", "encode2.txt");

	/*string temp = encoding.get_encode();*/

	//cout << endl << "Code-string: " << temp;

	/*encoding.output_file(filename_input2, true);*/

	encoding.parallel_huf_decode_text("encode2.txt", "decode2.txt", flag_ex);
	/*if (MASS1.size() < 2)
	{
		cout << endl << "Decode-string: " << text << endl;
	}*/



	if (rank == 0) {
	
		if (encoding.test("prpr2.txt", "decode2.txt") == 1) {
			cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
		}
		else cout << endl << "TEST: decoding is not correct" << endl;
	}
	delete[] frequencies;

	//---------------------------------------------------------------------------------------------------------------  RLE

	RLE rabbit;
	
	rabbit.RLEcode_parallel("prpr2.txt", "encRLE2.txt");
	rabbit.RLEdecode_parallel("encRLE2.txt", "decRLE2.txt");
	if (rank == 0) {
		cout << "  RLE: " << endl;
		
		//rabbit.output_file(filename_input5, false);
		if (rabbit.test("prpr2.txt", "decRLE2.txt") == 1) {
			cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
		}
		else cout << endl << "TEST: decoding is not correct" << endl;
	}
	//-------------------------------------------------------------------------------------------------------------- RLE + Huffman

	RLE wolf;
	huffman_coding fox;
	string temp3;
	int k = 10;
	string MASS;
	double* chance;
	bool flag_ex2;/*
	wolf.RLEcode_parallel(text);*/
	if (rank == 0) {
		cout << "  RLE+Huffman: " << endl << endl;
	}
		temp3 = rabbit.get_encode("encRLE2.txt");

		MASS.append(Mass);
		MASS.append("^");
		MASS.append("#");

		if (Max > 9) {
			for (int i = 9; i < Max; i++) {
				MASS += to_string(k);
				k++;
			}
		}
		chance = new double[MASS.size()];
		get_frequency(temp3, chance, MASS);
		/*cout << "c)  Frequencies: " << endl;*/
		/*for (int i = 0; i < MASS.size(); i++)
		{
			cout << MASS[i] << " - " << chance[i] << endl;
		}
		cout << endl;*/

		/* Обработка исключения: один символ */
		flag_ex2 = 0;
		if (MASS.size() < 2)
		{
			flag_ex2 = !flag_ex2;
		}

		fox.HuffmanCodes(MASS, chance, flag_ex2);
	
	fox.parallel_huf_encode_text("encRLE2.txt", "encRLEHUFF2.txt");

	/*string temp4 = fox.get_encode();*/

	//fox.output_file(filename_input6, true);
	

	fox.parallel_huf_decode_text("encRLEHUFF2.txt", "decRLEHUFF12.txt", flag_ex2);

	//if (wolf.test("encRLE.txt", "decRLEHUFF1.txt") == 1) {
	//	cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
	//}
	//else cout << endl << "TEST: decoding is not correct" << endl;

	wolf.RLEdecode_parallel("decRLEHUFF12.txt", "decRLEHUFF2.txt");
	//
	//	wolf.output_file(filename_input7, false);
	//
	if (rank == 0) {
		if (wolf.test("prpr2.txt", "decRLEHUFF2.txt") == 1) {
			cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
		}
		else cout << endl << "TEST: decoding is not correct" << endl;
	}

	delete[] chance;

	//	//--------------------------------------------------------------------------------------------------------------  Huffman + RLE
	//
	//cout << "  Huffman+RLE: " << endl << endl;
	RLE fish;
	huffman_coding bear;
	string MASS2;
	MASS2.append(Mass);
	double* chance2 = new double[MASS2.size()];;
	bool flag_ex5;
		get_frequency(text, chance2, MASS2);
		if (rank == 0) {
			cout << "  Huffman+RLE: " << endl << endl;
		}
		
		/*cout << "Frequencies: " << endl;
		for (int i = 0; i < (MASS2.size()); i++)
		{
			cout << MASS2[i] << " - " << chance2[i] << endl;

		}*/
		cout << endl;
		flag_ex5 = 0;
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

	//cout << "RANK: " << rank << " IS OK\n";
	fish.RLEcode_parallel("encode2.txt", "encHUFF+2.txt");
	//	string buf2 = fish.get_encode();
	//	/*cout << endl << "Code-string: " << rabbit.get_encode();*/
	//	fish.output_file(filename_input8, true);flag

	//
	fish.RLEdecode_parallel("encHUFF+2.txt", "decHUFF+12.txt");
	//
	//
	//
	//
	bear.parallel_huf_decode_text("decHUFF+12.txt", "decHUFF+2.txt", flag_ex5);
	//    bear.output_file(filename_input9, false);
	if (rank == 0) {
		if (bear.test("prpr2.txt", "decHUFF+2.txt") == 1) {
			cout << endl << "TEST: decoding is correct" << endl << "----------------------------------------------------------------------" << endl << endl;
		}
		else cout << endl << "TEST: decoding is not correct" << endl;
	}
	//
	delete[] chance2;
	Max = 0;

	
	
	MPI_Finalize();
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
