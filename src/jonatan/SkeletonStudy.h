#ifndef SKELETONSTUDY_H
#define SKELETONSTUDY_H

#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
 
#include "../globals.h"

#include "../AsyncStream.h"


#include "../RenderUtils.h"


 
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

	vector<float> aver;
	vector<float> suma;
	vector<float> maxim;
	vector<float> minim;
	float tot;
	

public:

		SkeletonStudy(Ptr<DataStream> baseStream) :
            base(baseStream)
        {
			float a=0;
			float b=100000;
			for(int i=0;i<19;i++){
				aver.push_back(a);
				suma.push_back(a);
				maxim.push_back(a);
				minim.push_back(b);
			}
			tot=0;

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

            ColorFrame* buffer = new ColorFrame();
			SkeletonFrame* skel = new SkeletonFrame();
            while (!stopping)
            {
				
                base->waitForFrame(buffer, nullptr, skel);

				SkeletonFrame skeleton;
				if ( base->getSkeletonFrame(skeleton)) {
					trackSkeleton(skeleton.frame);
				}
				
				

                pushFrame(buffer, nullptr, skel);
            }
            delete buffer;
			delete skel;

			
        }

		void trackSkeleton(const NUI_SKELETON_FRAME& frame);

		void saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos);

};

#endif