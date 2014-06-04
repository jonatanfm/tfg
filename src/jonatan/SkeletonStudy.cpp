
#include "SkeletonStudy.h"

void SkeletonStudy::trackSkeleton(const NUI_SKELETON_FRAME& frame){

	for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            
			tot++;
			saveBoneLength(frame.SkeletonData[i].SkeletonPositions[0],frame.SkeletonData[i].SkeletonPositions[1],0);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[1],frame.SkeletonData[i].SkeletonPositions[2],1);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[2],frame.SkeletonData[i].SkeletonPositions[3],2);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[2],frame.SkeletonData[i].SkeletonPositions[4],3);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[4],frame.SkeletonData[i].SkeletonPositions[5],4);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[5],frame.SkeletonData[i].SkeletonPositions[6],5);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[6],frame.SkeletonData[i].SkeletonPositions[7],6);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[2],frame.SkeletonData[i].SkeletonPositions[8],7);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[8],frame.SkeletonData[i].SkeletonPositions[9],8);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[9],frame.SkeletonData[i].SkeletonPositions[10],9);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[10],frame.SkeletonData[i].SkeletonPositions[11],10);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[0],frame.SkeletonData[i].SkeletonPositions[12],11);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[12],frame.SkeletonData[i].SkeletonPositions[13],12);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[13],frame.SkeletonData[i].SkeletonPositions[14],13);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[14],frame.SkeletonData[i].SkeletonPositions[15],14);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[0],frame.SkeletonData[i].SkeletonPositions[16],15);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[16],frame.SkeletonData[i].SkeletonPositions[17],16);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[17],frame.SkeletonData[i].SkeletonPositions[18],17);
            saveBoneLength(frame.SkeletonData[i].SkeletonPositions[18],frame.SkeletonData[i].SkeletonPositions[19],18);
		}
        else if (NUI_SKELETON_POSITION_ONLY == state) {

        }
    }

	
}

void SkeletonStudy::trackImprovedSkeleton(const NUI_SKELETON_FRAME& frame){

	vector<Vector4> positions;
	vector<bool> tracked;
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            
			tot++;
			
			for ( int k=0;k<20;k++){
				positions.push_back(frame.SkeletonData[i].SkeletonPositions[k]);
			}

			bool finish=false;
			//qDebug()<<frame.SkeletonData[i].eSkeletonPositionTrackingState[5]<<endl;
			while (!finish){
				finish=true;
				if (!checkSkeletonLength(positions[0],positions[1],0)) finish=false;
				if (!checkSkeletonLength(positions[1],positions[2],1)) finish=false;
				if (!checkSkeletonLength(positions[2],positions[3],2)) finish=false;
				if (!checkSkeletonLength(positions[2],positions[4],3)) finish=false;
				if (!checkSkeletonLength(positions[4],positions[5],4)) finish=false;
				if (!checkSkeletonLength(positions[5],positions[6],5)) finish=false;
				if (!checkSkeletonLength(positions[6],positions[7],6)) finish=false;
				if (!checkSkeletonLength(positions[2],positions[8],7)) finish=false;
				if (!checkSkeletonLength(positions[8],positions[9],8)) finish=false;
				if (!checkSkeletonLength(positions[9],positions[10],9)) finish=false;
				if (!checkSkeletonLength(positions[10],positions[11],10)) finish=false;
				if (!checkSkeletonLength(positions[0],positions[12],11)) finish=false;
				if (!checkSkeletonLength(positions[12],positions[13],12)) finish=false;
				if (!checkSkeletonLength(positions[13],positions[14],13)) finish=false;
				if (!checkSkeletonLength(positions[14],positions[15],14)) finish=false;
				if (!checkSkeletonLength(positions[0],positions[16],15)) finish=false;
				if (!checkSkeletonLength(positions[16],positions[17],16)) finish=false;
				if (!checkSkeletonLength(positions[17],positions[18],17)) finish=false;
				if (!checkSkeletonLength(positions[18],positions[19],18)) finish=false;
			}
			saveBoneLength(positions[0],positions[1],0);
            saveBoneLength(positions[1],positions[2],1);
            saveBoneLength(positions[2],positions[3],2);
            saveBoneLength(positions[2],positions[4],3);
            saveBoneLength(positions[4],positions[5],4);
            saveBoneLength(positions[5],positions[6],5);
            saveBoneLength(positions[6],positions[7],6);
            saveBoneLength(positions[2],positions[8],7);
            saveBoneLength(positions[8],positions[9],8);
            saveBoneLength(positions[9],positions[10],9);
            saveBoneLength(positions[10],positions[11],10);
            saveBoneLength(positions[0],positions[12],11);
            saveBoneLength(positions[12],positions[13],12);
            saveBoneLength(positions[13],positions[14],13);
            saveBoneLength(positions[14],positions[15],14);
            saveBoneLength(positions[0],positions[16],15);
            saveBoneLength(positions[16],positions[17],16);
            saveBoneLength(positions[17],positions[18],17);
            saveBoneLength(positions[18],positions[19],18);
		}
        else if (NUI_SKELETON_POSITION_ONLY == state) {

        }
    }
		
}


void SkeletonStudy::trackSkeletonPositions(const NUI_SKELETON_FRAME& frame) {
	for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            
			tot++;
			
			for (int k=0;k<20;k++){
				#ifdef HAS_LIBXL
					sheettmp[k]->writeNum(tot+2, 2, tot);
					sheettmp[k]->writeNum(2+tot, 3, frame.SkeletonData[i].SkeletonPositions[k].x);
					sheettmp[k]->writeNum(2+tot, 4, frame.SkeletonData[i].SkeletonPositions[k].y);
					sheettmp[k]->writeNum(2+tot, 5, frame.SkeletonData[i].SkeletonPositions[k].z);
					if(frame.SkeletonData[i].eSkeletonPositionTrackingState[k]==0)
						sheettmp[k]->writeStr(2+tot, 6, "not tracked");
					else if(frame.SkeletonData[i].eSkeletonPositionTrackingState[k]==1)
						sheettmp[k]->writeStr(2+tot, 6, "inferred");
					else if(frame.SkeletonData[i].eSkeletonPositionTrackingState[k]==2)
						sheettmp[k]->writeStr(2+tot, 6, "tracked");

				#endif
					
			}
		}
        else if (NUI_SKELETON_POSITION_ONLY == state) {

        }
    }

}

void SkeletonStudy::saveBoneLength(const Vector4& skel1,const Vector4& skel2,int pos){

	Point tmp = Point(skel2.x-skel1.x,skel2.y-skel1.y,skel2.z-skel1.z);

	float res = abs(sqrt((tmp.x*tmp.x) + (tmp.y*tmp.y) +(tmp.z*tmp.z)));

		
	#ifdef HAS_LIBXL
		sheettmp[pos]->writeNum(tot+2, 2, tot);
		sheettmp[pos]->writeNum(2+tot, 3, res);
		sheettmp[pos]->writeNum(2+tot, 4, aver[pos]);
		sheettmp[pos]->writeNum(2+tot, 5, aver[pos]+desv[pos]);
		sheettmp[pos]->writeNum(2+tot, 6, aver[pos]-desv[pos]);
	#endif
}

bool SkeletonStudy::checkSkeletonLength(Vector4& skel1,Vector4& skel2,int pos) {

	Point tmp = Point(skel2.x-skel1.x,skel2.y-skel1.y,skel2.z-skel1.z);

	float res = abs(sqrt((tmp.x*tmp.x) + (tmp.y*tmp.y) +(tmp.z*tmp.z)));

	tmp = Point(tmp.x/res,tmp.y/res,tmp.z/res);

	float d = res - aver[pos];

	if ( d < 0 && d < -desv[pos] ) {
		skel1.x=skel1.x+(tmp.x*(d/2));
		skel1.y=skel1.y+(tmp.y*(d/2));
		skel1.z=skel1.z+(tmp.z*(d/2));
		skel2.x=skel2.x-(tmp.x*(d/2));
		skel2.y=skel2.y-(tmp.y*(d/2));
		skel2.z=skel2.z-(tmp.z*(d/2));
	}else if ( d > 0 && d > desv[pos] ) {
		skel1.x=skel1.x+(tmp.x*(d/2));
		skel1.y=skel1.y+(tmp.y*(d/2));
		skel1.z=skel1.z+(tmp.z*(d/2));
		skel2.x=skel2.x-(tmp.x*(d/2));
		skel2.y=skel2.y-(tmp.y*(d/2));
		skel2.z=skel2.z-(tmp.z*(d/2));

	}
	else{
		return true;
	}
	
	return false;
}
