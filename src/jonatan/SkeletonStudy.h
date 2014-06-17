#ifndef SKELETONSTUDY_H
#define SKELETONSTUDY_H

#pragma once


#include <stdlib.h>

#include <direct.h>
#include <time.h>
#include "../globals.h"

#include "../AsyncStream.h"
#include "../RecordedStream.h"


#ifdef HAS_LIBXL

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

	bool done;
	vector<float> aver;
	vector<float> desv;
	float tot;
	int tot2;

	vector<vector<pair<Vector4,int> > > punts;
	vector<vector<pair<Vector4,int> > > puntsCorrected;

	bool redo;

	AsyncStream* s;
	RecordedStream* rec;

	#ifdef HAS_LIBXL

				Book* booktmp;
				vector<Sheet*> sheettmp;
			
	#endif


public:

		SkeletonStudy(Ptr<DataStream> baseStream, int typ, QString bones) :
            base(baseStream),
			type(typ)
        {
			
			tot=0;
			tot2=0;

			if(type==1 || type==3)	loadBonesData(bones);

			createSkeletonNames();
			createJointNames();
			redo=false;
			done=false;
			s = dynamic_cast<AsyncStream*>(base.obj);
			rec = dynamic_cast<RecordedStream*>(s);
			
            colorFrame = new ColorFrame();

			skeletonFrame = new SkeletonFrame();

            start();
        }
			
		std::string getName() const override
        {
            return "Tracking Skeleton in time";
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
							trackImprovedSkeleton(skeleton.frame,skel);
					if(rec->hasFinished()) stopping=true;
				}
				else if (type==2) {
					if(rec->hasFinished() && !redo){
						solveData();
						redo=true;
						rec->reset();
					}
					else if(rec->hasFinished() && redo && tot2>1){
						stopping=true;
					}


					if ( base->getSkeletonFrame(skeleton)) 
							trackSkeletonPositions(skeleton.frame,skel);
				}
				else if (type==3) {
					if(rec->hasFinished() && !redo){
						solveData();
						redo=true;
						rec->reset();
					}
					else if(rec->hasFinished() && redo  && tot2>1){
						
						stopping=true;
					}

					
					if ( base->getSkeletonFrame(skeleton)) 
							trackSkeletonPositionsChanged(skeleton.frame,skel);
					
					
				}
				
				
                pushFrame(buffer, nullptr, skel);
				
            }
			
			

			delete buffer;
			delete skel;

			QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
			QString filename,filename2;
			filename=filename2 = timestamp;
			filename2 += "_time.xls";

			QByteArray ba = filename2.toLocal8Bit();
			const char *filen = ba.data();

			#ifdef HAS_LIBXL
				if(type<2)booktmp->save(filen);       
				booktmp->release();
			#endif

			if (type>1){
				filename += "_dataintime";
				QByteArray ba2 = filename.toLocal8Bit();
				const char *filen2 = ba2.data();
				mkdir(filen2);
				

				for(int p=0;p<20;p++){
					ofstream outputFile1(string(filen2,21)+"/"+jointName[p]+"_data.csv");
					ofstream outputFile2(string(filen2,21)+"/"+jointName[p]+"_corrected.csv");
					for(int y=0;y<punts[0].size();y++){
						outputFile1 << punts[p][y].first.x<<","<< punts[p][y].first.y<<","<< punts[p][y].first.z<<","<< punts[p][y].second<<endl;
						outputFile2 << puntsCorrected[p][y].first.x<<","<< puntsCorrected[p][y].first.y<<","<< puntsCorrected[p][y].first.z<<","<< punts[p][y].second<<endl;
					}
					

					outputFile1.close();
					outputFile2.close();
				}
				createBVH();
			}
			
        }

		void trackSkeleton(const NUI_SKELETON_FRAME& frame);

		void trackImprovedSkeleton(const NUI_SKELETON_FRAME& frame,SkeletonFrame* frame2);

		void trackSkeletonPositions(const NUI_SKELETON_FRAME& frame,SkeletonFrame* frame2);

		void trackSkeletonPositionsChanged(const NUI_SKELETON_FRAME& frame,SkeletonFrame* frame2);

		void saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos);

		void solveData(){
			int f,s;
			Vector4 p0,p1,p2,p3;
			Vector4 p01,p31;
			pair<Vector4,int> res;
			bool find=false;
			vector<pair<Vector4,int> > as;
			for(int k=0;k<20;k++){
				as.push_back(punts[k][0]);
				puntsCorrected.push_back(as);
								
				for(int i=0;i<punts[k].size();i++){
					if(i==0){
					}
					else{//check final sin tracked
						if(punts[k][i].second!=2 && !find){
							
							find=true;
							f=i-1;
						}
						else if(punts[k][i].second==2 && find){
							
							find=false;
							s=i;
							p0.x=punts[k][f].first.x;p0.y=punts[k][f].first.y;p0.z=punts[k][f].first.z;
							p3.x=punts[k][s].first.x;p3.y=punts[k][s].first.y;p3.z=punts[k][s].first.z;

							p01.x=punts[k][f-1].first.x;p01.y=punts[k][f-1].first.y;p01.z=punts[k][f-1].first.z;
							p31.x=punts[k][s+1].first.x;p31.y=punts[k][s+1].first.y;p31.z=punts[k][s+1].first.z;
							p01.x=p0.x-p01.x;p01.y=p0.y-p01.y;p01.z=p0.z-p01.z;
							p31.x=p3.x-p31.x;p31.y=p3.y-p31.y;p31.z=p3.z-p31.z;
							
							

							float framerate=1;
							p1.x=p0.x+(framerate*p01.x);p1.y=p0.y+(framerate*p01.y);p1.z=p0.z+(framerate*p01.z);
							p2.x=p3.x-(framerate*p31.x);p2.y=p3.y-(framerate*p31.y);p2.z=p3.z-(framerate*p31.z);
							float d=s-f;

							float u=1.0/d;
							float m;
							

							for(int a=1;a<(s-f);a++){
								m=u*a;
								res.first.x = ( p0.x*pow((1-m),3) ) + (3 * p1.x *m* pow((1-m),2) ) + (3* p2.x * pow(m,2) * (1-m) ) + (p3.x*pow(m,3) );
								res.first.y = ( p0.y*pow((1-m),3) ) + (3 * p1.y *m* pow((1-m),2) ) + (3* p2.y * pow(m,2) * (1-m) ) + (p3.y*pow(m,3) );
								res.first.z = ( p0.z*pow((1-m),3) ) + (3 * p1.z *m* pow((1-m),2) ) + (3* p2.z * pow(m,2) * (1-m) ) + (p3.z*pow(m,3) );
								res.second=punts[k][f+1].second;
								
								puntsCorrected[k].push_back(res);
							}
							
							
							puntsCorrected[k].push_back(punts[k][i]);
						}
						else if(punts[k][i].second==2){
							puntsCorrected[k].push_back(punts[k][i]);
						}
					}
				}
			}
		}

		bool checkBackNextPoints(int k, int l){
			Point a=Point(0,0,0);
			for(int i = l-15;i<(l+15);i++){
				if(i!=l){
					if(punts[k][i].second!=2)return false;
					else{
						a.x+=punts[k][i].first.x;
						a.y+=punts[k][i].first.y;
						a.z+=punts[k][i].first.z;
					}
				}
			}
			pair<Vector4,int> s;
			s.first.x=a.x/30;
			s.first.y=a.y/30;
			s.first.z=a.z/30;
			s.second=2;
			puntsCorrected[k].push_back(s);
			return true;
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
			qDebug() <<filename<<endl;
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

		void SkeletonStudy::createBVH() {
			vector<pair<int,int> > joints;
			pair<int,int> ja;
			
			QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
			QString filename;
			filename = timestamp;
			QByteArray ba = filename.toLocal8Bit();
			const char *filen = ba.data();
			Vector4 tmp,tmp2,tmp3;
			ofstream outputFile(string(filen,10)+"_data.bvh");
			float bonesize = 100;
			outputFile << "HIERARCHY"<<endl;
			outputFile << "ROOT Hip"<<endl;
			outputFile << "{"<<endl;
			tmp=puntsCorrected[0][0].first;
			outputFile << "  OFFSET "<< tmp.x*bonesize << " " << tmp.y*bonesize <<" "<< tmp.z*bonesize<<endl;
			outputFile << "  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation"<<endl;
			outputFile << "  JOINT LeftHip"<<endl;
			outputFile << "  {"<<endl;
			ja.first=0;ja.second=12; joints.push_back(ja);
			tmp=puntsCorrected[0][0].first;tmp2=puntsCorrected[12][0].first;
			outputFile << "    OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "    CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "    JOINT LeftKnee"<<endl;
			outputFile << "    {"<<endl;
			ja.first=12;ja.second=13; joints.push_back(ja);
			tmp=puntsCorrected[12][0].first;tmp2=puntsCorrected[13][0].first;
			outputFile << "      OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "      CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "      JOINT LeftAnkle"<<endl;
			outputFile << "      {"<<endl;
			ja.first=13;ja.second=14; joints.push_back(ja);
			tmp=puntsCorrected[13][0].first;tmp2=puntsCorrected[14][0].first;
			outputFile << "        OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "        End Site"<<endl;
			outputFile << "        {"<<endl;
			ja.first=14;ja.second=15; joints.push_back(ja);
			tmp=puntsCorrected[14][0].first;tmp2=puntsCorrected[15][0].first;
			outputFile << "          OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        }"<<endl;
			outputFile << "      }"<<endl;
			outputFile << "    }"<<endl;
			outputFile << "  }"<<endl;
			outputFile << "  JOINT RightHip"<<endl;
			outputFile << "  {"<<endl;
			ja.first=0;ja.second=16; joints.push_back(ja);
			tmp=puntsCorrected[0][0].first;tmp2=puntsCorrected[16][0].first;
			outputFile << "    OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "    CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "    JOINT RightKnee"<<endl;
			outputFile << "    {"<<endl;
			ja.first=16;ja.second=17; joints.push_back(ja);
			tmp=puntsCorrected[16][0].first;tmp2=puntsCorrected[17][0].first;
			outputFile << "      OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "      CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "      JOINT RightAnkle"<<endl;
			outputFile << "      {"<<endl;
			ja.first=17;ja.second=18; joints.push_back(ja);
			tmp=puntsCorrected[17][0].first;tmp2=puntsCorrected[18][0].first;
			outputFile << "        OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "        End Site"<<endl;
			outputFile << "        {"<<endl;
			ja.first=18;ja.second=19; joints.push_back(ja);
			tmp=puntsCorrected[18][0].first;tmp2=puntsCorrected[19][0].first;
			outputFile << "          OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        }"<<endl;
			outputFile << "      }"<<endl;
			outputFile << "    }"<<endl;
			outputFile << "  }"<<endl;
			outputFile << "  JOINT Spine"<<endl;
			outputFile << "  {"<<endl;
			ja.first=0;ja.second=1; joints.push_back(ja);
			tmp=puntsCorrected[0][0].first;tmp2=puntsCorrected[1][0].first;
			outputFile << "    OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "    CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "    JOINT Neck"<<endl;
			outputFile << "    {"<<endl;
			ja.first=1;ja.second=2; joints.push_back(ja);
			tmp=puntsCorrected[1][0].first;tmp2=puntsCorrected[2][0].first;
			outputFile << "      OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "      CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "      JOINT LeftShoulder"<<endl;
			outputFile << "      {"<<endl;
			ja.first=2;ja.second=4; joints.push_back(ja);
			tmp=puntsCorrected[2][0].first;tmp2=puntsCorrected[4][0].first;
			outputFile << "        OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "        JOINT LeftElbow"<<endl;
			outputFile << "        {"<<endl;
			ja.first=4;ja.second=5; joints.push_back(ja);
			tmp=puntsCorrected[4][0].first;tmp2=puntsCorrected[5][0].first;
			outputFile << "          OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "          CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "          JOINT LeftWrist"<<endl;
			outputFile << "          {"<<endl;
			ja.first=5;ja.second=6; joints.push_back(ja);
			tmp=puntsCorrected[5][0].first;tmp2=puntsCorrected[6][0].first;
			outputFile << "            OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "            CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "            End Site"<<endl;
			outputFile << "            {"<<endl;
			ja.first=6;ja.second=7; joints.push_back(ja);
			tmp=puntsCorrected[6][0].first;tmp2=puntsCorrected[7][0].first;
			outputFile << "              OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "            }"<<endl;
			outputFile << "          }"<<endl;
			outputFile << "        }"<<endl;
			outputFile << "      }"<<endl;
			outputFile << "      JOINT RightShoulder"<<endl;
			outputFile << "      {"<<endl;
			ja.first=2;ja.second=8; joints.push_back(ja);
			tmp=puntsCorrected[2][0].first;tmp2=puntsCorrected[8][0].first;
			outputFile << "        OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "        JOINT RightElbow"<<endl;
			outputFile << "        {"<<endl;
			ja.first=8;ja.second=9; joints.push_back(ja);
			tmp=puntsCorrected[8][0].first;tmp2=puntsCorrected[9][0].first;
			outputFile << "          OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "          CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "          JOINT RightWrist"<<endl;
			outputFile << "          {"<<endl;
			ja.first=9;ja.second=10; joints.push_back(ja);
			tmp=puntsCorrected[9][0].first;tmp2=puntsCorrected[10][0].first;
			outputFile << "            OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "            CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "            End Site"<<endl;
			outputFile << "            {"<<endl;
			ja.first=10;ja.second=11; joints.push_back(ja);
			tmp=puntsCorrected[10][0].first;tmp2=puntsCorrected[11][0].first;
			outputFile << "              OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "            }"<<endl;
			outputFile << "          }"<<endl;
			outputFile << "        }"<<endl;
			outputFile << "      }"<<endl;
			outputFile << "      JOINT Head"<<endl;
			outputFile << "      {"<<endl;
			ja.first=2;ja.second=3; joints.push_back(ja);
			tmp=puntsCorrected[2][0].first;tmp2=puntsCorrected[3][0].first;
			outputFile << "        OFFSET "<< (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize <<endl;
			outputFile << "        CHANNELS 3 Zrotation Xrotation Yrotation"<<endl;
			outputFile << "      }"<<endl;
			outputFile << "    }"<<endl;
			outputFile << "  }"<<endl;
			outputFile << "}"<<endl;
			outputFile << "MOTION"<<endl;
			outputFile << "Frames:	"<< puntsCorrected[0].size() <<endl;
			outputFile << "Frame Time:	0.0333333"<<endl;
			for(int h=1;h<puntsCorrected[0].size();h++){
				outputFile << puntsCorrected[0][h].first.x << " "<< puntsCorrected[0][h].first.y << " "<< puntsCorrected[0][h].first.z << " ";
				for(int qw=0;qw<joints.size();qw++){
					tmp=puntsCorrected[joints[qw].first][h].first;
					tmp2=puntsCorrected[joints[qw].second][h].first;
					tmp.x=tmp.x-tmp2.x;tmp.y=tmp.y-tmp2.y;tmp.z=tmp.z-tmp2.z;
					tmp2=puntsCorrected[joints[qw].first][h-1].first;
					tmp3=puntsCorrected[joints[qw].second][h-1].first;
					tmp2.x=tmp2.x-tmp3.x;tmp2.y=tmp2.y-tmp3.y;tmp2.z=tmp2.z-tmp3.z;

					outputFile << (tmp2.x-tmp.x)*bonesize <<" "<< (tmp2.y-tmp.y)*bonesize <<" "<< (tmp2.z-tmp.z)*bonesize;
					if((qw+1) <joints.size())outputFile<<" ";
				}
				outputFile << endl;
			}
			
			
			outputFile.close();


		};
};

#endif