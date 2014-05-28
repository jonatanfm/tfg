#ifndef SKELETONSTUDY_H
#define SKELETONSTUDY_H

#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>


 #include <time.h>
#include "../globals.h"

#include "../AsyncStream.h"


#include "../RenderUtils.h"


#ifndef WITH_LIBXL

#include "libxl.h"
using namespace libxl;

#endif

using namespace std;


struct Point
{
    float x, y, z;

    inline Point() { }
    inline Point(float x, float y, float z) : x(x), y(y), z(z) { }
};

class SkeletonStudy : public AsyncStream
{

private:

    Ptr<DataStream> base;

	vector<const char*> skelName;
	vector<float> aver;
	vector<float> desv;
	vector<float> suma;
	vector<float> sumaCuadrada;
	vector<float> maxim;
	vector<float> minim;
	float tot;
	
	AsyncStream* s;
	#ifndef WITH_LIBXL

				Book* booktmp;
				Sheet* sheettmp;
			
	#endif


public:

		SkeletonStudy(Ptr<DataStream> baseStream) :
            base(baseStream)
        {
			
			float a=0;
			float b=100000;
			for(int i=0;i<19;i++){
				aver.push_back(a);
				desv.push_back(a);
				suma.push_back(a);
				sumaCuadrada.push_back(a);
				maxim.push_back(a);
				minim.push_back(b);
			}
			tot=0;
			createSkeletonNames();
			 s = dynamic_cast<AsyncStream*>(base.obj);

            colorFrame = new ColorFrame();

			skeletonFrame = new SkeletonFrame();

            start();
        }

		std::string getName() const override
        {
            return "Traking Skeleton and saving data to file";
        }

		bool isOpened() const override
        {
            return true;
        }

        void stream() override
        {
			
			
			SkeletonFrame skeleton;
            ColorFrame* buffer = new ColorFrame();
			SkeletonFrame* skel = new SkeletonFrame();
			
			#ifndef WITH_LIBXL

				booktmp = xlCreateBook();
				sheettmp = booktmp->addSheet("Kinect Skeleton in time");
				Format* centerFormat = booktmp->addFormat();
				centerFormat->setAlignH(ALIGNH_CENTER);
				sheettmp->writeStr(2, 0, "Left arm");
				sheettmp->writeStr(2, 1, "Right arm");
				for(int i=0; i<19;i++){
					sheettmp->setCol(2, 2+i, 20);
					sheettmp->writeStr(2, 2+i, skelName[i],centerFormat);
				}

			#endif

				while (!stopping)
            {
				
				if (s==NULL) stopping=true;


                base->waitForFrame(buffer, nullptr, skel);

				
				if ( base->getSkeletonFrame(skeleton)) {
					trackSkeleton(skeleton.frame);
				}

				
				
                pushFrame(buffer, nullptr, skel);
            }
			
			delete buffer;
			delete skel;

			QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
			QString filename;
			filename = timestamp;
			filename += "_results.xls";
			QString filename2;
			filename2 = timestamp;
			filename2 += "_time.xls";

			QByteArray ba = filename.toLocal8Bit();
			QByteArray ba2 = filename2.toLocal8Bit();
			const char *filen = ba.data();
			const char *filen2 = ba2.data();

			#ifndef WITH_LIBXL
				booktmp->save(filen2);       
				booktmp->release();
				

			#endif

			#ifndef WITH_LIBXL

				Book* book = xlCreateBook();
				Sheet* sheet = book->addSheet("Kinect Skeleton Values");
			
				if(sheet)
				{
					
					Format* centerFormat = book->addFormat();
					centerFormat->setAlignH(ALIGNH_CENTER);

					sheet->setCol(1, 1, 20);

					sheet->writeStr(3, 1, "Average",centerFormat);
					sheet->writeStr(4, 1, "Deviation",centerFormat);
					sheet->writeStr(5, 1, "Max",centerFormat);
					sheet->writeStr(6, 1, "Min",centerFormat);
					for(int i=0; i<19;i++){
						
						sheet->setCol(2, 2+i, 20);

						sheet->writeStr(2, 2+i, skelName[i],centerFormat);
						sheet->writeNum(3, 2+i, aver[i],centerFormat);
						sheet->writeNum(4, 2+i, desv[i],centerFormat);
						sheet->writeNum(5, 2+i, maxim[i],centerFormat);
						sheet->writeNum(6, 2+i, minim[i],centerFormat);
					}

					
				}
				
				book->save(filen);       
				book->release();

			#endif
			
        }

		void trackSkeleton(const NUI_SKELETON_FRAME& frame);

		void saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos);

		float getAngle(const Vector4& skel1,const Vector4& skel2,const Vector4& skel3);

		void changeState() {
			stopping = true;
		}

		void createSkeletonNames(){

			skelName.push_back("Hip-Spine");//0
			skelName.push_back("Spine-Shoulders");//1
			skelName.push_back("Shoulders-Head");//2
			skelName.push_back("Shoulders-LShoulder");//3
			skelName.push_back("LShoulder-LElbow");//4
			skelName.push_back("LElbow-LWrist");//5
			skelName.push_back("LWrist-LHand");//6
			skelName.push_back("Shoulders-RShoulder");//7
			skelName.push_back("RShoulder-RElbow");//8
			skelName.push_back("RElbow-RWrist");//9
			skelName.push_back("RWrist-RHand");//10
			skelName.push_back("Hip-LHip");//11
			skelName.push_back("LHip-LKnee");//12
			skelName.push_back("LKnee-LAnkle");//13
			skelName.push_back("LAnkle-LFoot");//14
			skelName.push_back("Hip-RHip");//15
			skelName.push_back("RHip-RKnee");//16
			skelName.push_back("RKnee-RAnkle");//17
			skelName.push_back("RAnkle-RFoot");//18
		
		}

};

#endif