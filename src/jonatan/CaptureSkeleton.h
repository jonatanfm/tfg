#ifndef CAPTURESKELETON_H
#define CAPTURESKELETON_H

#pragma once

#include <stdlib.h>


#include <time.h>
#include "../globals.h"

#include "../AsyncStream.h"




#ifdef HAS_LIBXL

#include "libxl.h"
using namespace libxl;

#endif

using namespace std;


struct Joint
{
    float x, y, z;

    inline Joint() { }
    inline Joint(float x, float y, float z) : x(x), y(y), z(z) { }
};

class CaptureSkeleton : public AsyncStream
{
    private:
        Ptr<DataStream> base;

        float radiusLength;
		float tibiaLength;

		AsyncStream* s;

		vector<const char*> skelName;
		vector<float> aver;
		vector<float> desv;
		vector<float> suma;
		vector<float> sumaCuadrada;
		vector<float> maxim;
		vector<float> minim;
		float tot;

public:	

		CaptureSkeleton(Ptr<DataStream> baseStream,float ra,float ti) :
            base(baseStream),
			radiusLength(ra),
			tibiaLength(ti)
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
            return "Traking Skeleton Lengths";
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
		
			
            
		#ifdef HAS_LIBXL
			QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
			QString filename;
			filename = timestamp;
			filename += "_data.xls";

			QByteArray ba = filename.toLocal8Bit();
			const char *filen = ba.data();

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
						desv[i] = sqrt( (sumaCuadrada[i]/tot)-(aver[i]*aver[i]) );
						sheet->writeStr(2, 2+i, skelName[i],centerFormat);
						sheet->writeNum(3, 2+i, aver[i],centerFormat);
						sheet->writeNum(4, 2+i, desv[i],centerFormat);
						sheet->writeNum(5, 2+i, maxim[i],centerFormat);
						sheet->writeNum(6, 2+i, minim[i],centerFormat);
					}
					radiusLength = 0.272f;
					tibiaLength = 0.4096f;
					if(radiusLength!= 0 && tibiaLength !=0){
						if(checkBoneLength(sheet,centerFormat)){
							//for reescribir media
						}
					}
					
				}
				
				book->save(filen);       
				book->release();

			#endif
		}


		void trackSkeleton(const NUI_SKELETON_FRAME& frame);

		void saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos);

		float getAngle(const Vector4& skel1,const Vector4& skel2,const Vector4& skel3);


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

		bool checkBoneLength(Sheet* sheet,Format* centerFormat){

			bool boneChange = false;
			float h,h1,h2;
			h1=radiusLength/0.146f;
			h2=tibiaLength/0.246f;

			h= (h1+h2)/2.0f;
			sheet->writeNum(2,0,radiusLength,centerFormat);
			sheet->writeNum(3,0,tibiaLength,centerFormat);
			sheet->writeNum(4,0,h,centerFormat);
			float a,b;
			vector<float> values;
			
			values.push_back(0.0f);values.push_back(0.0f);values.push_back(0.118f);
			values.push_back(0.129f);values.push_back(0.166f);values.push_back(0.146f);values.push_back(0.058f);
			values.push_back(0.129f);values.push_back(0.166f);values.push_back(0.146f);values.push_back(0.058f);
			values.push_back(0.0f);values.push_back(0.245f);values.push_back(0.246f);values.push_back(0.039f);
			values.push_back(0.0f);values.push_back(0.245f);values.push_back(0.246f);values.push_back(0.039f);

			/*a=aver[0]+aver[1]+desv[0]+desv[1];
			b=aver[0]+aver[1]-desv[0]-desv[0];
			if(h*0.288 <b || a < h*0.288){
				boneChange=true;
				float perc = aver[0]/(aver[0]+aver[1]);
				sheet->writeNum(3, 2, h*0.288*perc,centerFormat);
				sheet->writeNum(3, 2+1, h*0.288*(1-perc),centerFormat);
			}
			sheet->writeNum(8,2,h*0.288,centerFormat);
			*/

			for(int i = 2; i<19;i++){
				sheet->writeNum(9,2+i,aver[i],centerFormat);
				if(values[i]!=0){
					a=aver[i]+desv[i];
					b=aver[i]-desv[i];
					if(h*values[i] <b || a < h*values[i]){
						boneChange=true;
						sheet->writeNum(3, 2+i, h*values[i],centerFormat);
					}
				}
			}

			return boneChange;
		
		}


};

#endif