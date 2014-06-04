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
	vector<const char*> jointName;

	vector<float> aver;
	vector<float> desv;
	float tot;
	
	AsyncStream* s;
	#ifdef HAS_LIBXL

				Book* booktmp;
				vector<Sheet*> sheettmp;
			
	#endif


public:

		SkeletonStudy(Ptr<DataStream> baseStream,int typ, QString bones) :
            base(baseStream),
			type(typ)
        {
			
			tot=0;
			
			loadBonesData(bones);

			createSkeletonNames();
			createJointNames();
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
								
				Format* centerFormat = booktmp->addFormat();
				centerFormat->setAlignH(ALIGNH_CENTER);
				
			if(type<2)
				for(int i=0; i<19;i++){
					sheettmp.push_back(booktmp->addSheet(skelName[i]));
					sheettmp[i]->setCol(2, 2, 30);
					sheettmp[i]->setCol(2, 3, 30);
					sheettmp[i]->setCol(2, 4, 30);
					sheettmp[i]->setCol(2, 5, 30);
					sheettmp[i]->setCol(2, 6, 30);
					sheettmp[i]->writeStr(2, 3, "Length",centerFormat);
					sheettmp[i]->writeStr(2, 4, "Mean",centerFormat);
					sheettmp[i]->writeStr(2, 5, "Mean+desv",centerFormat);
					sheettmp[i]->writeStr(2, 6, "Mean-desv",centerFormat);
				}
			else
				for (int i=0; i<20;i++){
					sheettmp.push_back(booktmp->addSheet(jointName[i]));
					sheettmp[i]->setCol(2, 2, 30);
					sheettmp[i]->setCol(2, 3, 30);
					sheettmp[i]->setCol(2, 4, 30);
					sheettmp[i]->setCol(2, 5, 30);
					sheettmp[i]->setCol(2, 6, 30);
					sheettmp[i]->writeStr(2, 3, "X",centerFormat);
					sheettmp[i]->writeStr(2, 4, "Y",centerFormat);
					sheettmp[i]->writeStr(2, 5, "Z",centerFormat);
					sheettmp[i]->writeStr(2, 6, "Tracked/Inherit",centerFormat);
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
				else if (type==1) {
					if ( base->getSkeletonFrame(skeleton) ) 
							trackImprovedSkeleton(skeleton.frame);
				}
				else if (type==2) {
					if ( base->getSkeletonFrame(skeleton) ) 
							trackSkeletonPositions(skeleton.frame);
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

		void trackImprovedSkeleton(const NUI_SKELETON_FRAME& frame);

		void trackSkeletonPositions(const NUI_SKELETON_FRAME& frame);

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

		void createJointNames(){

			jointName.push_back("Hip");//0
			jointName.push_back("Spine");//1
			jointName.push_back("Shoulders");//2
			jointName.push_back("Head");//3
			jointName.push_back("LShoulder");//4
			jointName.push_back("LElbow");//5
			jointName.push_back("LWrist");//6
			jointName.push_back("LHand");//7
			jointName.push_back("RShoulder");//8
			jointName.push_back("RElbow");//9
			jointName.push_back("RWrist");//10
			jointName.push_back("RHand");//11
			jointName.push_back("LHip");//12
			jointName.push_back("LKnee");//13
			jointName.push_back("LAnkle");//14
			jointName.push_back("LFoot");//15
			jointName.push_back("RHip");//16
			jointName.push_back("RKnee");//17
			jointName.push_back("RAnkle");//18
			jointName.push_back("RFoot");//19
		
		}

		void loadBonesData(QString bones) {
			QByteArray ba = bones.toLocal8Bit();
			const char *filename = ba.data();

			Book* book = xlCreateBook();
			
			if(book->load(filename))
			   {
				  Sheet* sheet = book->getSheet(0);
				  if(sheet)
				  {
					 for(int i=0; i<19; ++i)
					 {
						aver.push_back(sheet->readNum(3, 2+i));
						desv.push_back(sheet->readNum(4, 2+i));
					 }
				  }
			   }

			 book->release();
		}

		bool SkeletonStudy::checkSkeletonLength(Vector4& skel1,Vector4& skel2,int pos);
};

#endif