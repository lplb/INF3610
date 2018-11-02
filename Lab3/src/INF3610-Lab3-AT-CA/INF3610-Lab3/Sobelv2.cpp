///////////////////////////////////////////////////////////////////////////////
//
//	Sobelv2.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include "Sobelv2.h"


///////////////////////////////////////////////////////////////////////////////
//
//	Constructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobelv2::Sobelv2( sc_module_name name ) 
	: sc_module(name)
/* À compléter */
{
	/*
	
	À compléter
	
	*/
	SC_THREAD(thread);
	sensitive << clk;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Destructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobelv2::~Sobelv2()
{
	/*
	
	À compléter
	
	*/
}


///////////////////////////////////////////////////////////////////////////////
//
//	thread
//
///////////////////////////////////////////////////////////////////////////////
void Sobelv2::thread(void)
{
	/*
	
	À compléter
	
	*/
	// Variable
	unsigned int imgWidth, imgHeight;
	unsigned int tempAddress = 0;

	while (true) {
		address.write(tempAddress);
		requestRead.write(true);
		do {
			wait(clk->posedge_event());
		} while (!ackReaderWriter.read());

		imgWidth = dataRW.read();

		tempAddress += 4;
		address.write(tempAddress);
		requestRead.write(true);
		do {
			wait(clk->posedge_event());
		} while (!ackReaderWriter.read());

		imgHeight = dataRW.read();
		requestRead.write(false);

		unsigned int imgSize = imgWidth * imgHeight;

		//Create array
		uint8_t * image = new uint8_t[imgSize];
		uint8_t * result = new uint8_t[imgSize];
		int * imageAsInt = reinterpret_cast<int*>(image);
		int * resultAsInt = reinterpret_cast<int*>(result);

		//For simplicity, assume that the borders don't contain edges
		for (unsigned int i = 0; i < imgWidth; ++i)
			result[i] = 0;
		for (unsigned int i = imgSize - imgWidth; i < imgSize; ++i)
			result[i] = 0;
		for (unsigned int i = 0; i < imgSize; i += imgWidth)
			result[i] = 0;
		for (unsigned int i = imgWidth - 1; i < imgSize; i += imgWidth)
			result[i] = 0;

		tempAddress = 4;
		//Calling the operator for each pixel
		for (unsigned int i = 1; i < imgHeight - 1; ++i) {
			for (unsigned int j = 1; j < imgWidth - 1; ++j) {
				int fullIndex = i * imgWidth + j;
				//Request element
				tempAddress += 4;
				address.write(tempAddress);
				requestRead.write(true);
				do {
					wait(clk->posedge_event());
				} while (!ackReaderWriter.read());

				imageAsInt[fullIndex] = dataRW.read();
				requestRead.write(false);
			}

			if (i >= 2)
			{
				for (unsigned int j = 1; j < imgWidth - 1; ++j) {
					int fullIndex = (i-1) * imgWidth + j;
					result[fullIndex] = Sobelv2_operator(fullIndex, imgWidth, image);

					/**************************				//TODO wait(); avec bon parametre ********************************************/
					/**************************				//TODO wait(); avec bon parametre ********************************************/
					/**************************				//TODO wait(); avec bon parametre ********************************************/
					/**************************				//TODO wait(); avec bon parametre ********************************************/
					/**************************				//TODO wait(); avec bon parametre ********************************************/
				}
			}
		}

		//Derniere ligne (excluant bord)
		for (unsigned int j = 1; j < imgWidth - 1; ++j) {
			int fullIndex = (imgHeight - 1) * imgWidth + j;
			result[fullIndex] = Sobelv2_operator(fullIndex, imgWidth, image);

			/**************************				//TODO wait(); avec bon parametre ********************************************/
			/**************************				//TODO wait(); avec bon parametre ********************************************/
			/**************************				//TODO wait(); avec bon parametre ********************************************/
			/**************************				//TODO wait(); avec bon parametre ********************************************/
			/**************************				//TODO wait(); avec bon parametre ********************************************/
		}

		//Write back nb. elements at the end
		tempAddress = 4;
		for (unsigned int i = 0; i < imgSize / sizeof(int); i++) {
			//Write each element
			tempAddress += 4;
			address.write(tempAddress);
			dataRW.write(resultAsInt[i]);
			requestWrite.write(true);
			do {
				wait(clk->posedge_event());
			} while (!ackReaderWriter.read());
			requestWrite.write(false);
		}

		delete(image);
		delete(result);
		sc_stop();
		wait();

	}

}

///////////////////////////////////////////////////////////////////////////////
//
//	Sobelv2_operator
//
///////////////////////////////////////////////////////////////////////////////
static inline uint8_t getVal(int index, int xDiff, int yDiff, int img_width, uint8_t * Y)
{
	int fullIndex = (index + (yDiff * img_width)) + xDiff;
	if (fullIndex < 0)
	{
		//Cas ou on doit chercher la derniere ligne
		fullIndex += img_width * 4;
	}
	else if (fullIndex >= img_width * 4)
	{
		//Cas ou on doit aller chercher la premiere ligne
		fullIndex -= img_width * 4;
	}

	return Y[fullIndex];
};

uint8_t Sobelv2::Sobelv2_operator(const int index, const int imgWidth, uint8_t * image)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	const char x_op[3][3] = { { -1,0,1 },
	{ -2,0,2 },
	{ -1,0,1 } };

	const char y_op[3][3] = { { 1,2,1 },
	{ 0,0,0 },
	{ -1,-2,-1 } };

	//Compute approximation of the gradients in the X-Y direction
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// X direction gradient
			x_weight = x_weight + (getVal(index, i - 1, j - 1, imgWidth, image) * x_op[i][j]);

			// Y direction gradient
			y_weight = y_weight + (getVal(index, i - 1, j - 1, imgWidth, image) * y_op[i][j]);
		}
	}

	edge_weight = std::abs(x_weight) + std::abs(y_weight);

	edge_val = (255 - (uint8_t)(edge_weight));

	//Edge thresholding
	if (edge_val > 200)
		edge_val = 255;
	else if (edge_val < 100)
		edge_val = 0;

	return edge_val;
}