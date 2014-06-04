#ifndef SKELETONSTUDY_H
#define SKELETONSTUDY_H

#pragma once


#include <stdlib.h>


 #include <time.h>
#include "../globals.h"

#include "../AsyncStream.h"




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
	int type;

	vector<const char*> skelName;
	vector<float> aver;
	vector<float> desv;
	float tot;
	
	AsyncStream* s;
	#ifdef HAS_LIBXL

				Book* booktmp;
				Sheet* sheettmp;
			
	#endif


public:

		SkeletonStudy(Ptr<DataStream> baseStream) :
            base(baseStream),
			type(0)
        {
			
			tot=0;
			createSkeletonNames();
			s = dynamic_cast<AsyncStream*>(base.obj);

            colorFrame = new ColorFrame();

			skeletonFrame = new SkeletonFrame();

            start();
        }

		std::string getName() const override
        {
            return "Traking Skeleton in time";
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
			
			#ifdef HAS_LIBXL

				booktmp = xlCreateBook();
				sheettmp = booktmp->addSheet("Kinect Skeleton in time");
				Format* centerFormat = booktmp->addFormat();
				centerFormat->setAlignH(ALIGNH_CENTER);
				
				for(int i=0; i<19;i++){
					sheettmp->setCol(2, 2+i, 20);
					sheettmp->writeStr(2, 2+i, skelName[i],centerFormat);
				}

			#endif

				while (!stopping)
            {
				
				if (s==NULL) stopping=true;


                base->waitForFrame(buffer, nullptr, skel);

				if (type==0){
					if ( base->getSkeletonFrame(skeleton)) 
							trackSkeleton(skeleton.frame);
				}
						
				
                pushFrame(buffer, nullptr, skel);
            }
			
			delete buffer;
			delete skel;

			QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
			QString filename;
			filename = timestamp;
			filename += "_time.xls";

			QByteArray ba = filename.toLocal8Bit();
			const char *filen = ba.data();

			#ifdef HAS_LIBXL
				booktmp->save(filen);       
				booktmp->release();
				

			#endif

			
        }

		void trackSkeleton(const NUI_SKELETON_FRAME& frame);

		void saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos);


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